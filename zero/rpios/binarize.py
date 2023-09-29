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
    """
    # スクリプトが存在するディレクトリのパスを取得
    script_directory = os.path.dirname(os.path.abspath(__file__))

    # スクリプトが存在するディレクトリ内の全てのファイルをリストする
    files = os.listdir(script_directory)

    processed = False  # 画像が処理されたかどうかをチェックするフラグ

    for file in files:
        full_path = os.path.join(script_directory, file)
        if file.lower().endswith('.jpg') or file.lower().endswith('.jpeg') or file.lower().endswith('.png'):
            print(f"Processing {file}...")

            with Image.open(full_path).convert("L") as img:
                width, height = img.size  # 画像のサイズ
                total_pixels = width * height  # 全ピクセル数を計算

                # 画像を二値化
                processed_img, white_pixels = binarize_pixels(img, threshold=30)

                # 白いピクセルの割合を計算
                white_ratio = calculate_white_ratio(white_pixels, total_pixels)

                # 新しいファイル名を生成
                base_name, ext = os.path.splitext(file)
                new_file_name = f"{base_name}_binarize{ext}"

                # 処理した画像を保存
                save_image(processed_img, new_file_name)

                print(f"Saved as {new_file_name}")
                print(f"White pixel ratio: {white_ratio:.2f}%")

            processed = True

    if not processed:
        print("No suitable image files found for processing.")"""
