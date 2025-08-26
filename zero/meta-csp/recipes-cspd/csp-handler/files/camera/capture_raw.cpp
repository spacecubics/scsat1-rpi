/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "capture_raw.h"
#include "../camera.h"

#include <libcamera/libcamera.h>
#include <libcamera/framebuffer.h>
#include <chrono>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <condition_variable>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <systemd/sd-journal.h>

#include "dng_writer.h"
#include "frame_plane_mapper.h"

#define CAM_FRAME_TIMEOUT 4

using namespace libcamera;

extern "C" {

std::condition_variable cv;
std::mutex mutex;
bool isPhotoCaptured = false;

static std::string getNextFilename(const std::string &directory, const std::string &prefix,
				   const std::string &extension)
{
	unsigned maxNum = 0;

	DIR *dp = opendir(directory.c_str());
	if (!dp) {
		sd_journal_print(LOG_ERR, "Failed to open dir %s\n", directory.c_str());
		return "";
	}

	struct dirent *entry;
	while ((entry = readdir(dp)) != nullptr) {
		unsigned num;
		if (sscanf(entry->d_name, (prefix + "-%u." + extension).c_str(), &num) == 1) {
			if (num > maxNum) {
				maxNum = num;
			}
		}
	}
	closedir(dp);

	std::ostringstream filename;
	filename << directory << "/" << prefix << "-" << std::setfill('0') << std::setw(3)
		 << (maxNum + 1) << "." << extension;
	return filename.str();
}

int writeBuffer(const Camera *camera, const Stream *stream, FrameBuffer *buffer,
		const ControlList &metadata, std::string filename)
{
	int ret = 0;

	if (filename.empty()) {
		sd_journal_print(LOG_ERR, "Filename empty\n");
		return -1;
	}

	/* Map the FrameBuffer to memory and retrieve the raw data pointer */ 
	std::unique_ptr<FramePlaneMapper> framePlane = FramePlaneMapper::fromFrameBuffer(buffer);
	if (!framePlane) {
		sd_journal_print(LOG_ERR, "Failed to map buffer to FramePlaneMapper\n");
		return -1;
	}

	const uint8_t *rawData = framePlane->data(0);

	/* Write the DNG file */
	ret = DNGWriter::write(filename.c_str(), camera, stream->configuration(), metadata,
			       rawData);

	if (ret < 0) {
		sd_journal_print(LOG_ERR, "Failed to write DNG file : %s\n", filename.c_str());
	}

	return ret;
}

static void requestComplete(Request *request)
{
	if (request->status() == Request::RequestCancelled) {
		return;
	}

	std::lock_guard<std::mutex> lock(mutex);
	isPhotoCaptured = true;
	cv.notify_one();
}

static void cleanup(std::shared_ptr<Camera> camera, CameraManager &manager)
{
	if (camera) {
		camera->stop();
		camera->release();
	}
	manager.stop();
	isPhotoCaptured = false;
}

int captureRawImage()
{
	CameraManager manager;
	isPhotoCaptured = false;
	int ret;

	if (manager.start()) {
		sd_journal_print(LOG_ERR, "Unable to start the camera manager\n");
		return -1;
	}

	if (manager.cameras().empty()) {
		sd_journal_print(LOG_ERR, "No cameras detected\n");
		cleanup(nullptr, manager);
		return -1;
	}

	std::string cameraId = manager.cameras()[0]->id();
	auto camera = manager.get(cameraId);

	if (camera->acquire() != 0) {
		sd_journal_print(LOG_ERR, "Unable to acquire the camera\n");
		cleanup(camera, manager);
		return -1;
	}

	std::unique_ptr<CameraConfiguration> config =
		camera->generateConfiguration({StreamRole::Raw});
	if (!config || config->size() == 0) {
		sd_journal_print(LOG_ERR, "Unable to configure the camera\n");
		cleanup(camera, manager);
		return -1;
	}

	config->at(0).pixelFormat = formats::SGRBG10_CSI2P;
	config->at(0).size = {CAMERA_WIDTH, CAMERA_HEIGHT};

	if (config->validate() == CameraConfiguration::Invalid) {
		sd_journal_print(LOG_ERR, "Invalid configuration\n");
		cleanup(camera, manager);
		return -1;
	}

	if (camera->configure(config.get()) != 0) {
		sd_journal_print(LOG_ERR, "Configuration failed\n");
		cleanup(camera, manager);
		return -1;
	}

	FrameBufferAllocator allocator(camera);
	for (StreamConfiguration &streamConfig : *config) {
		if (allocator.allocate(streamConfig.stream()) < 0) {
			sd_journal_print(LOG_ERR, "Frame buffer allocation failed\n");
			cleanup(camera, manager);
			return -1;
		}
	}

	Stream *stream = config->at(0).stream();
	std::vector<std::unique_ptr<Request>> requests;

	for (auto &buffer : allocator.buffers(stream)) {
		std::unique_ptr<Request> request = camera->createRequest();

		ControlList &controls = request->controls();
		controls.set(controls::AeEnable, true);
		controls.set(controls::AwbEnable, true);
		controls.set(controls::Brightness, CAMERA_DEFAULT_BRIGTHNESS);

		request->addBuffer(stream, buffer.get());
		requests.push_back(std::move(request));
	}

	camera->requestCompleted.connect(requestComplete);

	if (camera->start()) {
		sd_journal_print(LOG_ERR, "Failed to start the camera\n");
		cleanup(camera, manager);
		return -1;
	}

	if (camera->queueRequest(requests[0].get())) {
		sd_journal_print(LOG_ERR, "Unable to queue the request\n");
		cleanup(camera, manager);
		return -1;
	}

	std::unique_lock<std::mutex> lock(mutex);
	if (!cv.wait_for(lock, std::chrono::seconds(CAM_FRAME_TIMEOUT),
			 [] { return isPhotoCaptured; })) {
		sd_journal_print(LOG_ERR, "Timeout frame capture\n");
		cleanup(camera, manager);
		return -1;
	}

	FrameBuffer *buffer = requests[0]->buffers().begin()->second;
	if (buffer == nullptr) {
		sd_journal_print(LOG_ERR, "No buffer available\n");
		cleanup(camera, manager);
		return -1;
	}

	std::string filename =
		getNextFilename(CAM_FRAME_PATH, CAM_FRAME_PREFIX, CAM_FRAME_EXTENSION);
	ret = writeBuffer(camera.get(), requests[0]->buffers().begin()->first,
		    requests[0]->buffers().begin()->second, requests[0]->metadata(), filename);

	cleanup(camera, manager);
	return ret;
}

} /* extern "C" */
