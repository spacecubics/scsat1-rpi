/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CAPTURE_RAW_H
#define CAPTURE_RAW_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Capture a raw image from the camera and save it to a file.
 *
 * The function captures an image using libcamera and writes it to a file in the directory specified
 * by CAM_FRAME_PATH. It automatically generates the filename using the CAM_FRAME_PREFIX and
 * CAM_FRAME_EXTENSION constants.
 *
 * @return 0 on success, or a non-zero error code on failure.
 */
int captureRawImage();

#ifdef __cplusplus
}
#endif

#endif // CAPTURE_RAW_H
