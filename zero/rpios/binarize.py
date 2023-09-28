from PIL import Image
import os
import exifrw


# 画像内の各ピクセルを二値化（白または黒）にする関数。白いピクセルの数も計算します。
def binarize_pixels(file_path, threshold=30):
    with Image.open(file_path).convert("L") as img:
        pixels = img.load()  # ピクセルデータをロード
        white_pixels = 0
        width, height = img.size  # 画像のサイズ
        for x in range(width):
            for y in range(height):
                if pixels[x, y] <= threshold:
                    pixels[x, y] = 0  # 黒にする
                else:
                    pixels[x, y] = 255  # 白にする
                    white_pixels += 1  # 白いピクセルの数をカウント
        # 画像のファイルサイズを取得
        total_pixels = width * height  # 全ピクセル数を計算
        white_ratio = (white_pixels / total_pixels) * 100  # 白いピクセルの割合を計算
    return img, white_pixels, white_ratio


# 白いピクセルの比率を計算する関数。
def get_file_size(img):
    file_size = os.path.getsize(img)
    return file_size


def save_image(img, path):
    img.save(path)


def binarize_xmp(file_path='./ImageJPG/image1.jpg'):
    # exifrw.pyのxmp_readを使って，全ての画像のxmpを読み込む
    xmp_data = exifrw.xmp_get_dict(file_path)
    # xmpの中に"BinalizedWhiteRate"タグがない画像を見つける
    if "BinalizedWhiteRate" not in xmp_data:
    # その画像をbinalize_pixelsで二値化し，calculate_white_ratioで白いピクセルの割合を計算
        _, _, white_ratio = binarize_pixels(file_path)
        # 白いピクセルの割合を"BinalizedWhiteRate"タグに書き込む
        exifrw.xmp_write(file_path, "BinalizedWhiteRate", str(white_ratio))
        # 現在処理中のファイル名を表示
        print(f"Processing {file_path}...")
        # 追加したタグを表示
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
