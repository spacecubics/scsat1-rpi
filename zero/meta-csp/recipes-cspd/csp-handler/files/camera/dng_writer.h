/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi Ltd
 *
 * DNG writer
 */

#pragma once

#include <libcamera/camera.h>
#include <libcamera/controls.h>
#include <libcamera/stream.h>

enum CFAPatternColour : uint8_t {
	CFAPatternRed = 0,
	CFAPatternGreen = 1,
	CFAPatternBlue = 2,
};

class DNGWriter
{
      public:
	static int write(const char *filename, const libcamera::Camera *camera,
			 const libcamera::StreamConfiguration &config,
			 const libcamera::ControlList &metadata, const void *data);
};
