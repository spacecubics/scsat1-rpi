import pyexiv2
import os
import sys
from io import StringIO

IMG_PATH = "./ImageJPG/image1.jpg"


def exif_read(path=""):
    with pyexiv2.Image(path) as img:
        data = img.read_exif()
        print("----------------")
        print(type(data))
        for key, value in data.items():
            print(f'{key}: {value}')
        print("----------------")


def xmp_read(path=""):
    with pyexiv2.Image(path) as img:
        data = img.read_xmp()
        print("----------------")
        print(type(data))
        for key, value in data.items():
            print(f'{key}: {value}')
        print("----------------")


def xmp_get_dict(path=""):
    with pyexiv2.Image(path) as img:
        data = img.read_xmp()
        return data


def xmp_write(path="", tag="", data=""):
    pyexiv2.registerNs("namespace for scsat1-rpi", "sc1")
    img = pyexiv2.Image(path)
    if tag == "":
        pass
    else:
        tag = "Xmp.sc1." + tag
        img.modify_xmp({tag: data})


def xmp_delete(path=""):
    key = input("Are you sure to delete all xmp data? (y/n): ")
    if key == "y":
        with pyexiv2.Image(path) as img:
            img.clear_xmp()
    else:
        pass


def capture_output(func, *args, **kwargs):
    """Utility function to capture print output from a function."""
    old_stdout = sys.stdout
    sys.stdout = new_stdout = StringIO()
    try:
        func(*args, **kwargs)
    finally:
        sys.stdout = old_stdout
    return new_stdout.getvalue()


def meta_save(image_path):
    """Reads metadata from the image and saves it to a .txt file."""
    # Capturing the output from the exif_read and xmp_read functions
    exif_output = capture_output(exif_read, image_path)
    xmp_output = capture_output(xmp_read, image_path)

    # Combining both outputs
    full_output = exif_output + "\n" + xmp_output

    # Generating the output file name
    output_filename = os.path.splitext(image_path)[0] + ".txt"

    # Writing the combined output to the .txt file
    with open(output_filename, "w") as file:
        file.write(full_output)

    return f"Metadata saved to {output_filename}"


def meta_save_directory(directory_path="./ImageJPG"):
    """Reads metadata from all .jpg files in the directory and saves it to .txt files."""
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            if file.lower().endswith('.jpg'):
                file_path = os.path.join(root, file)
                meta_save(file_path)
    return f"Metadata saved for all .jpg files in {directory_path}"


if __name__ == "__main__":
    # xmp_delete(IMG_PATH)
    # exif_read(IMG_PATH)
    # xmp_read(IMG_PATH)
    # xmp_write(IMG_PATH, "BinalizedWhiteRate", "100"); xmp_read(IMG_PATH)
    meta_save_directory()
