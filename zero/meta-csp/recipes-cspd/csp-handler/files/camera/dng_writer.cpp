/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi Ltd
 *
 * DNG writer
 */

#include "dng_writer.h"

#include <algorithm>
#include <tiffio.h>
#include <libcamera/control_ids.h>
#include <libcamera/formats.h>
#include <libcamera/property_ids.h>
#include <systemd/sd-journal.h>

#define DNG_COMPANY "Space Cubics, LLC."
#define DNG_SOFTWARE "SC-SAT1"

/*
* Pick a reasonable number eps to protect against singularities. It
* should be comfortably larger than the point at which we run into
* numerical trouble, yet smaller than any plausible gain that we might
* apply to a colour, either explicitly or as part of the colour matrix.
*/
constexpr float EPSILON = 1e-2;
constexpr unsigned int THUMBNAIL_DOWNSCALE_FACTOR = 16;
constexpr uint8_t DNG_VERSION[] = {1, 2, 0, 0};
constexpr uint8_t JPEG_BIT_PER_SAMPLE = 8;
constexpr uint8_t SGRBG10_CSI2P_BIT_PER_SAMPLE = 10;
constexpr CFAPatternColour SGRBG10_CSI2P_COLOUR_PATTERN[] = {CFAPatternGreen, CFAPatternRed, CFAPatternBlue, CFAPatternGreen};
constexpr uint16_t CFA_REPEAT_PATTERN_DIM[] = {2, 2};
constexpr uint8_t CFA_PLANE_COLOR[] = {CFAPatternRed, CFAPatternGreen, CFAPatternBlue};

using namespace libcamera;

struct Matrix3d {
	Matrix3d() = default;

	Matrix3d(float m0, float m1, float m2, float m3, float m4, float m5, float m6, float m7,
		 float m8)
	{
		m[0] = m0;
		m[1] = m1;
		m[2] = m2;
		m[3] = m3;
		m[4] = m4;
		m[5] = m5;
		m[6] = m6;
		m[7] = m7;
		m[8] = m8;
	}

	Matrix3d(const Span<const float> &span)
	{
		for (int i = 0; i < 9; ++i) {
			m[i] = span[i];
		}
	}

	static Matrix3d diag(float diag0, float diag1, float diag2)
	{
		return {diag0, 0, 0, 0, diag1, 0, 0, 0, diag2};
	}

	static Matrix3d identity()
	{
		return diag(1.0f, 1.0f, 1.0f);
	}

	Matrix3d transpose() const
	{
		return {m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8]};
	}

	Matrix3d cofactors() const
	{
		return {m[4] * m[8] - m[5] * m[7], -(m[3] * m[8] - m[5] * m[6]),
			m[3] * m[7] - m[4] * m[6], -(m[1] * m[8] - m[2] * m[7]),
			m[0] * m[8] - m[2] * m[6], -(m[0] * m[7] - m[1] * m[6]),
			m[1] * m[5] - m[2] * m[4], -(m[0] * m[5] - m[2] * m[3]),
			m[0] * m[4] - m[1] * m[3]};
	}

	Matrix3d adjugate() const
	{
		return cofactors().transpose();
	}

	float determinant() const
	{
		return m[0] * (m[4] * m[8] - m[5] * m[7]) - m[1] * (m[3] * m[8] - m[5] * m[6]) +
		       m[2] * (m[3] * m[7] - m[4] * m[6]);
	}

	Matrix3d inverse() const
	{
		return adjugate() * (1.0f / determinant());
	}

	Matrix3d operator*(const Matrix3d &other) const
	{
		Matrix3d result;
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				result.m[i * 3 + j] = 0;
				for (int k = 0; k < 3; ++k) {
					result.m[i * 3 + j] += m[i * 3 + k] * other.m[k * 3 + j];
				}
			}
		}
		return result;
	}

	Matrix3d operator*(float f) const
	{
		Matrix3d result;
		for (int i = 0; i < 9; ++i) {
			result.m[i] = m[i] * f;
		}
		return result;
	}

	float m[9]{};
};

void packScanlineSGRBG10_CSI2P(void *output, const void *input, unsigned int width)
{
	const uint8_t *in = static_cast<const uint8_t *>(input);
	uint8_t *out = static_cast<uint8_t *>(output);

	for (unsigned int x = 0; x < width; x += 4) {
		*out++ = in[0];
		*out++ = (in[4] & 0x03) << 6 | in[1] >> 2;
		*out++ = (in[1] & 0x03) << 6 | (in[4] & 0x0c) << 2 | in[2] >> 4;
		*out++ = (in[2] & 0x0f) << 4 | (in[4] & 0x30) >> 2 | in[3] >> 6;
		*out++ = (in[3] & 0x3f) << 2 | (in[4] & 0xc0) >> 6;
		in += 5;
	}
}

void thumbScanlineSGRBG10_CSI2P(void *output, const void *input,
			   unsigned int width, unsigned int stride)
{
	const uint8_t *in = static_cast<const uint8_t *>(input);
	uint8_t *out = static_cast<uint8_t *>(output);

	/* Number of bytes corresponding to 16 pixels. */
	unsigned int skip = SGRBG10_CSI2P_BIT_PER_SAMPLE * 16 / 8;

	for (unsigned int x = 0; x < width; x++) {
		uint8_t value = (in[0] + in[1] + in[stride] + in[stride + 1]) >> 2;
		*out++ = value;
		*out++ = value;
		*out++ = value;
		in += skip;
	}
}

/*
 * Tags that apply to the whole file are stored here.
 */
static void setCommonTiffTags(TIFF *tif, const ControlList &cameraProperties)
{
	TIFFSetField(tif, TIFFTAG_DNGVERSION, DNG_VERSION);
	TIFFSetField(tif, TIFFTAG_DNGBACKWARDVERSION, DNG_VERSION);
	TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(tif, TIFFTAG_MAKE, DNG_COMPANY);

	const auto &model = cameraProperties.get(properties::Model);
	if (model) {
		TIFFSetField(tif, TIFFTAG_MODEL, model->c_str());
	}

	TIFFSetField(tif, TIFFTAG_SOFTWARE, DNG_SOFTWARE);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
}

/*
 * Thumbnail-specific tags. The thumbnail is stored as an RGB image
 * with 1/16 of the raw image resolution. Greyscale would save space,
 * but doesn't seem well supported by RawTherapee.
 */
static void setThumbnailTiffTags(TIFF *tif, const StreamConfiguration &config)
{
	TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE);
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, config.size.width / THUMBNAIL_DOWNSCALE_FACTOR);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, config.size.height / THUMBNAIL_DOWNSCALE_FACTOR);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, JPEG_BIT_PER_SAMPLE);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
}

static int writeThumbnail(TIFF *tif, const StreamConfiguration &config, const void *data,
			  const ControlList &metadata)
{
	uint8_t scanline[(config.size.width * SGRBG10_CSI2P_BIT_PER_SAMPLE + 7) / 8];

	toff_t rawIFDOffset = 0;
	toff_t exifIFDOffset = 0;

	/* Tag for thumbnail */
	setThumbnailTiffTags(tif, config);

	/*
	 * Fill in some reasonable colour information in the DNG. We supply
	 * the "neutral" colour values which determine the white balance, and the
	 * "ColorMatrix1" which converts XYZ to (un-white-balanced) camera RGB.
	 * Note that this is not a "proper" colour calibration for the DNG,
	 * nonetheless, many tools should be able to render the colours better.
	 */
	float neutral[3] = {1, 1, 1};
	Matrix3d wbGain = Matrix3d::identity();
	/* From http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html */
	const Matrix3d rgb2xyz(0.4124564, 0.3575761, 0.1804375, 0.2126729, 0.7151522, 0.0721750,
			       0.0193339, 0.1191920, 0.9503041);
	Matrix3d ccm = Matrix3d::identity();

	const auto &colourGains = metadata.get(controls::ColourGains);
	if (colourGains) {
		if ((*colourGains)[0] > EPSILON && (*colourGains)[1] > EPSILON) {
			wbGain = Matrix3d::diag((*colourGains)[0], 1, (*colourGains)[1]);
			neutral[0] = 1.0 / (*colourGains)[0]; /* red */
			neutral[2] = 1.0 / (*colourGains)[1]; /* blue */
		}
	}

	const auto &ccmControl = metadata.get(controls::ColourCorrectionMatrix);
	if (ccmControl) {
		Matrix3d ccmSupplied(*ccmControl);
		if (ccmSupplied.determinant() > EPSILON) {
			ccm = ccmSupplied;
		}
	}

	/*
	 * rgb2xyz is known to be invertible, and we've ensured above that both
	 * the ccm and wbGain matrices are non-singular, so the product of all
	 * three is guaranteed to be invertible too.
	 */
	Matrix3d colorMatrix1 = (rgb2xyz * ccm * wbGain).inverse();

	TIFFSetField(tif, TIFFTAG_COLORMATRIX1, 9, colorMatrix1.m);
	TIFFSetField(tif, TIFFTAG_ASSHOTNEUTRAL, 3, neutral);

	/*
	 * Reserve space for the SubIFD and ExifIFD tags, pointing to the IFD
	 * for the raw image and EXIF data respectively. The real offsets will
	 * be set later.
	 */
	TIFFSetField(tif, TIFFTAG_SUBIFD, 1, &rawIFDOffset);
	TIFFSetField(tif, TIFFTAG_EXIFIFD, exifIFDOffset);

	/* Write thumbnail data */
	const uint8_t *row = static_cast<const uint8_t *>(data);
	for (unsigned int y = 0; y < config.size.height / THUMBNAIL_DOWNSCALE_FACTOR; y++) {
		thumbScanlineSGRBG10_CSI2P(&scanline, row,
						config.size.width / THUMBNAIL_DOWNSCALE_FACTOR,
						config.stride);

		if (TIFFWriteScanline(tif, &scanline, y, 0) != 1) {
			return -EINVAL;
		}

		row += config.stride * THUMBNAIL_DOWNSCALE_FACTOR;
	}

	TIFFWriteDirectory(tif);
	return 0;
}

static void setRawImageTiffTag(TIFF *tif, const StreamConfiguration &config)
{
	/* Tag for raw data */
	TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0);
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, config.size.width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, config.size.height);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, SGRBG10_CSI2P_BIT_PER_SAMPLE);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_CFA);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(tif, TIFFTAG_CFAREPEATPATTERNDIM, CFA_REPEAT_PATTERN_DIM);
	TIFFSetField(tif, TIFFTAG_CFAPLANECOLOR, 3, CFA_PLANE_COLOR);

	if (TIFFLIB_VERSION < 20201219) {
		TIFFSetField(tif, TIFFTAG_CFAPATTERN, SGRBG10_CSI2P_COLOUR_PATTERN);
	} else {
		TIFFSetField(tif, TIFFTAG_CFAPATTERN, 4, SGRBG10_CSI2P_COLOUR_PATTERN);
	}

	TIFFSetField(tif, TIFFTAG_CFAPLANECOLOR, 3, CFA_PLANE_COLOR);
	TIFFSetField(tif, TIFFTAG_CFALAYOUT, 1);
}

static void setWhiteAndBlackLevel(TIFF *tif, const ControlList &metadata)
{
	const uint16_t blackLevelRepeatDim[] = {2, 2};
	float blackLevel[] = {0.0f, 0.0f, 0.0f, 0.0f};
	uint32_t whiteLevel = (1 << SGRBG10_CSI2P_BIT_PER_SAMPLE) - 1;

	const auto &blackLevels = metadata.get(controls::SensorBlackLevels);
	if (blackLevels) {
		Span<const int32_t, 4> levels = *blackLevels;
		/*
		 * The black levels control is specified in R, Gr, Gb, B order.
		 * Map it to the TIFF tag that is specified in CFA pattern
		 * order.
		 */
		unsigned int green = (SGRBG10_CSI2P_COLOUR_PATTERN[0] == CFAPatternRed ||
				      SGRBG10_CSI2P_COLOUR_PATTERN[1] == CFAPatternRed)
					     ? 0
					     : 1;

		for (unsigned int i = 0; i < 4; ++i) {
			unsigned int level;

			switch (SGRBG10_CSI2P_COLOUR_PATTERN[i]) {
			case CFAPatternRed:
				level = levels[0];
				break;
			case CFAPatternGreen:
				level = levels[green + 1];
				green = (green + 1) % 2;
				break;
			case CFAPatternBlue:
			default:
				level = levels[3];
				break;
			}
			/* Map the 16-bit value to the bits per sample range. */
			blackLevel[i] = level >> (16 - SGRBG10_CSI2P_BIT_PER_SAMPLE);
		}
	}

	TIFFSetField(tif, TIFFTAG_BLACKLEVELREPEATDIM, &blackLevelRepeatDim);
	TIFFSetField(tif, TIFFTAG_BLACKLEVEL, 4, &blackLevel);
	TIFFSetField(tif, TIFFTAG_WHITELEVEL, 1, &whiteLevel);
}

static void setRawImageTimeData(TIFF *tif)
{
	/* Store creation time. */
	time_t rawtime;
	struct tm *timeinfo;
	char strTime[20];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(strTime, 20, "%Y:%m:%d %H:%M:%S", timeinfo);

	TIFFSetField(tif, EXIFTAG_DATETIMEORIGINAL, strTime);
	TIFFSetField(tif, EXIFTAG_DATETIMEDIGITIZED, strTime);
}

static int writeRawImage(TIFF *tif, const StreamConfiguration &config, const void *data,
			 const ControlList &metadata)
{
	uint8_t scanline[(config.size.width * SGRBG10_CSI2P_BIT_PER_SAMPLE + 7) / 8];

	toff_t rawIFDOffset = 0;
	toff_t exifIFDOffset = 0;

	setRawImageTiffTag(tif, config);
	setWhiteAndBlackLevel(tif, metadata);

	/* write raw image */
	const uint8_t *row = static_cast<const uint8_t *>(data);
	for (unsigned int y = 0; y < config.size.height; y++) {
		packScanlineSGRBG10_CSI2P(&scanline, row, config.size.width);

		if (TIFFWriteScanline(tif, &scanline, y, 0) != 1) {
			return -EINVAL;
		}

		row += config.stride;
	}

	/* Checkpoint the IFD to retrieve its offset, and write it out. */
	TIFFCheckpointDirectory(tif);
	rawIFDOffset = TIFFCurrentDirOffset(tif);
	TIFFWriteDirectory(tif);

	/* Create a new IFD for the EXIF data and fill it. */
	TIFFCreateEXIFDirectory(tif);

	setRawImageTimeData(tif);

	const auto &analogGain = metadata.get(controls::AnalogueGain);
	if (analogGain) {
		uint16_t iso = std::min(std::max(*analogGain * 100, 0.0f), 65535.0f);
		TIFFSetField(tif, EXIFTAG_ISOSPEEDRATINGS, 1, &iso);
	}

	const auto &exposureTime = metadata.get(controls::ExposureTime);
	if (exposureTime) {
		TIFFSetField(tif, EXIFTAG_EXPOSURETIME, *exposureTime / 1e6);
	}

	TIFFWriteCustomDirectory(tif, &exifIFDOffset);

	/* Update the IFD offsets and close the file. */
	TIFFSetDirectory(tif, 0);
	TIFFSetField(tif, TIFFTAG_SUBIFD, 1, &rawIFDOffset);
	TIFFSetField(tif, TIFFTAG_EXIFIFD, exifIFDOffset);

	TIFFWriteDirectory(tif);
	return 0;
}

int DNGWriter::write(const char *filename, const Camera *camera, const StreamConfiguration &config,
		     const ControlList &metadata, const void *data)
{
	const ControlList &cameraProperties = camera->properties();

	TIFF *tif = TIFFOpen(filename, "w");
	if (!tif) {
		sd_journal_print(LOG_ERR, "Failed to open tiff file\n");
		return -EINVAL;
	}

	setCommonTiffTags(tif, cameraProperties);

	if (writeThumbnail(tif, config, data, metadata) < 0) {
		TIFFClose(tif);
		sd_journal_print(LOG_ERR, "Failed to write thumbnail\n");
		return -EINVAL;
	}

	if (writeRawImage(tif, config, data, metadata) < 0) {
		TIFFClose(tif);
		sd_journal_print(LOG_ERR, "Failed to write RAW image\n");
		return -EINVAL;
	}

	TIFFClose(tif);
	return 0;
}
