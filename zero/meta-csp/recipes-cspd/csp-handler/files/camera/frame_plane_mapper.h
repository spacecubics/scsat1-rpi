/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <stdint.h>
#include <vector>

#include <libcamera/base/span.h>
#include <libcamera/framebuffer.h>

class FramePlaneMapper
{
      public:
	static std::unique_ptr<FramePlaneMapper>
	fromFrameBuffer(const libcamera::FrameBuffer *buffer);

	~FramePlaneMapper();

	const uint8_t *data(unsigned int plane) const;

      private:
	FramePlaneMapper();
	std::vector<libcamera::Span<const uint8_t>> planes_;
};
