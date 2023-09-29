#!/usr/bin/python3

import time
from picamera2 import Picamera2
import parameters as param  # Assuming parameters.py contains common parameters
import file


def capture_dng_jpg(output_filename="image0", folderin=True):
    picam2 = Picamera2()

    exposure = param.metadata["ExposureTime"]  # Read from parameters module
    gain = param.metadata["AnalogueGain"] * param.metadata["DigitalGain"]  # Read from parameters module

    controls = {"ExposureTime": exposure, "AnalogueGain": gain}
    capture_config = picam2.create_still_configuration(raw={}, display=None, controls=controls)

    picam2.start()
    time.sleep(2)

    r = picam2.switch_mode_capture_request_and_stop(capture_config)

    if folderin == False:
        r.save("main", output_filename + ".jpg")
        r.save_dng(output_filename + ".dng")

    else:
        r.save("main", "ImageJPG/" + output_filename + ".jpg")
        r.save_dng("ImageDNG/" + output_filename + ".dng")

    picam2.stop()


def capture_dng(output_filename="cap_dng", folderin=True):
    picam2 = Picamera2()

    exposure = param.metadata["ExposureTime"]  # Read from parameters module
    gain = param.metadata["AnalogueGain"] * param.metadata["DigitalGain"]  # Read from parameters module

    controls = {"ExposureTime": exposure, "AnalogueGain": gain}
    capture_config = picam2.create_still_configuration(raw={}, display=None, controls=controls)

    picam2.start()
    time.sleep(2)

    if folderin == False:
        r = picam2.switch_mode_capture_request_and_stop(capture_config)
        r.save_dng(output_filename)

    else:
        r = picam2.switch_mode_capture_request_and_stop(capture_config)
        r.save_dng("ImageDNG/" + output_filename + ".dng")

    picam2.stop()


def capture_jpg(output_filename="cap_jpg", folderin=True):
    picam2 = Picamera2()

    exposure = param.metadata["ExposureTime"]  # Read from parameters module
    gain = param.metadata["AnalogueGain"] * param.metadata["DigitalGain"]  # Read from parameters module

    controls = {"ExposureTime": exposure, "AnalogueGain": gain}
    capture_config = picam2.create_still_configuration(raw={}, display=None, controls=controls)

    # picam2.configure(capture_config)  # maybe needless
    picam2.start()
    time.sleep(2)

    if folderin == False:
        r = picam2.switch_mode_capture_request_and_stop(capture_config)
        r.save("main", output_filename + ".jpg")

    else:
        r = picam2.switch_mode_capture_request_and_stop(capture_config)
        r.save("main", "ImageJPG/" + output_filename + ".jpg")

    picam2.stop()


if __name__ == "__main__":
    capture_dng_jpg(file.get_name())
