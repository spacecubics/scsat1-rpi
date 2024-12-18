/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame_plane_mapper.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <systemd/sd-journal.h>

using namespace libcamera;

std::unique_ptr<FramePlaneMapper> FramePlaneMapper::fromFrameBuffer(const FrameBuffer *buffer)
{
	std::unique_ptr<FramePlaneMapper> image{new FramePlaneMapper()};

	if (buffer->planes().empty()) {
		sd_journal_print(LOG_ERR, "FrameBuffer has no planes\n");
		return nullptr;
	}

	for (const FrameBuffer::Plane &plane : buffer->planes()) {
		const int fd = plane.fd.get();
		const size_t length = lseek(fd, 0, SEEK_END);
		if (plane.offset > length || plane.offset + plane.length > length) {
			sd_journal_print(LOG_ERR,
					 "Plane is out of buffer: buffer length= %zu, plane "
					 "offset=%u, plane length=%u\n",
					 length, plane.offset, plane.length);
			return nullptr;
		}

		void *address =
			mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, fd, plane.offset);
		if (address == MAP_FAILED) {
			sd_journal_print(LOG_ERR, "Failed to mmap plane: %s\n", strerror(errno));
			return nullptr;
		}

		image->planes_.emplace_back(static_cast<const uint8_t *>(address), plane.length);
	}

	return image;
}

FramePlaneMapper::FramePlaneMapper() = default;

FramePlaneMapper::~FramePlaneMapper()
{
	for (const auto &plane : planes_) {
		munmap(const_cast<uint8_t *>(plane.data()), plane.size());
	}
}

const uint8_t *FramePlaneMapper::data(unsigned int plane) const
{
	assert(plane < planes_.size());
	return planes_[plane].data();
}
