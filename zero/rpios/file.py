import os
import glob


# 次に使用する画像の名前を生成する関数
def get_name():
    jpg_files = glob.glob("ImageJPG/*.jpg")
    dng_files = glob.glob("ImageDNG/*.dng")

    jpg_numbers = [int(os.path.basename(f).replace("image", "").replace(".jpg", "")) for f in jpg_files]
    dng_numbers = [int(os.path.basename(f).replace("image", "").replace(".dng", "")) for f in dng_files]

    all_numbers = set(jpg_numbers) | set(dng_numbers)

    if not all_numbers:
        return "image1"

    return f"image{max(all_numbers) + 1}"


# 片方のディレクトリにしか存在しないファイルを削除する関数
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


# ImageJPG と ImageDNG の画像ファイルのリストを表示する関数
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

# 指定された番号の .jpg と .dng 画像ファイルを削除する関数
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

if __name__ == "__main__":
    list_file()
    print(get_name())
