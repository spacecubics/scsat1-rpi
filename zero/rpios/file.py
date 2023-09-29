import os
import glob


# Function to generate the name of the next image to use
def get_name():
    jpg_files = glob.glob("ImageJPG/*.jpg")
    dng_files = glob.glob("ImageDNG/*.dng")

    jpg_numbers = [int(os.path.basename(f).replace("image", "").replace(".jpg", "")) for f in jpg_files]
    dng_numbers = [int(os.path.basename(f).replace("image", "").replace(".dng", "")) for f in dng_files]

    all_numbers = set(jpg_numbers) | set(dng_numbers)

    if not all_numbers:
        return "image1"

    return f"image{max(all_numbers) + 1}"


# Function to remove files that exist only in one directory
def format_number():
    jpg_files = glob.glob("ImageJPG/*.jpg")
    dng_files = glob.glob("ImageDNG/*.dng")

    jpg_numbers = {int(os.path.basename(f).replace("image", "").replace(".jpg", "")) for f in jpg_files}
    dng_numbers = {int(os.path.basename(f).replace("image", "").replace(".dng", "")) for f in dng_files}

    only_jpg = jpg_numbers - dng_numbers
    only_dng = dng_numbers - jpg_numbers

    for num in only_jpg:
        os.remove(f"ImageJPG/image{num}.jpg")

    for num in only_dng:
        os.remove(f"ImageDNG/image{num}.dng")


# Function to display a list of image files in ImageJPG and ImageDNG
def list_file():
    jpg_files = glob.glob("ImageJPG/*.jpg")
    dng_files = glob.glob("ImageDNG/*.dng")

    print("\nImageJPG:")
    for f in jpg_files:
        print(os.path.basename(f))

    print("ImageDNG:")
    for f in dng_files:
        print(os.path.basename(f))
    print("\n")

# Function to delete the specified .jpg and .dng image files by number
def delete_file(number:int):
    jpg_file = f"ImageJPG/image{number}.jpg"
    dng_file = f"ImageDNG/image{number}.dng"

    if os.path.exists(jpg_file) and os.path.exists(dng_file):
        os.remove(jpg_file)
        os.remove(dng_file)
    elif os.path.exists(jpg_file):
        print(f"Error: {dng_file} does not exist.")
    elif os.path.exists(dng_file):
        print(f"Error: {jpg_file} does not exist.")
    else:
        print(f"Error: Both {jpg_file} and {dng_file} do not exist.")


def loop_in_directory(function, directory_path='./ImageJPG'):
    """
    Apply a given function to all .jpg files in the specified directory.

    Args:
    - function (function): The function to apply to each .jpg file.
    - directory_path (str): Path to the directory containing .jpg files.
    """
    # List all files in the directory
    file_list = os.listdir(directory_path)

    # Filter out non-jpg files
    jpg_files = [f for f in file_list if f.lower().endswith('.jpg')]

    if not jpg_files:
        print("No jpg images found in the directory.")
        return

    # Process each jpg file
    for filename in jpg_files:
        file_path = os.path.join(directory_path, filename)
        function(file_path)



if __name__ == "__main__":
    list_file()
    print(get_name())
