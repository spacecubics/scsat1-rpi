from PIL import Image
import os
import exifrw


# Function to binarize each pixel in the image (white or black). It also calculates the number of white pixels.
def binarize_pixels(file_path, threshold=30):
    with Image.open(file_path).convert("L") as img:
        pixels = img.load()  # 	Load pixel data
        white_pixels = 0
        width, height = img.size  # Load pixel data
        for x in range(width):
            for y in range(height):
                if pixels[x, y] <= threshold:
                    pixels[x, y] = 0  # Make it black
                else:
                    pixels[x, y] = 255  # Make it white
                white_pixels += 1  # Count the number of white pixels
        # Get the image file size
        total_pixels = width * height  # Calculate the total number of pixels
        white_ratio = (white_pixels / total_pixels) * 100  # Calculate the proportion of white pixels
    return img, white_pixels, white_ratio


# Function to get file size
def get_file_size(img):
    file_size = os.path.getsize(img)
    return file_size


def save_image(img, path):
    img.save(path)


def binarize_xmp(file_path='./ImageJPG/image1.jpg'):
    # Use xmp_read from exifrw.py to read the xmp of all images
    xmp_data = exifrw.xmp_get_dict(file_path)
    # Find images that do not have the "BinalizedWhiteRate" tag in xmp
    if "BinalizedWhiteRate" not in xmp_data:
        # Binarize that image with binarize_pixels and calculate the proportion of white pixels with calculate_white_ratio
        _, _, white_ratio = binarize_pixels(file_path)
        # Write the proportion of white pixels to the "BinalizedWhiteRate" tag
        exifrw.xmp_write(file_path, "BinalizedWhiteRate", str(white_ratio))
        # Display the filename currently being processed
        print(f"Processing {file_path}...")
        # Display the added tag
        print(exifrw.xmp_get_dict(file_path))
    else:
        pass
    return f"Processed {file_path} with BinalizedWhiteRate: {white_ratio}"


if __name__ == "__main__":
    binarize_xmp("./ImageJPG/image1.jpg")
