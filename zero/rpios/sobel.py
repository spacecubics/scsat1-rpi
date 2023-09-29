import cv2
import numpy as np
import exifrw

def sobel_variance(image_path):
    # Load the image
    image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)

    # Check if the image was loaded properly
    if image is None:
        print(f"Error: Could not open or find the image at {image_path}")
        return None

    # Apply the Sobel filter
    sobelx = cv2.Sobel(image, cv2.CV_64F, 1, 0, ksize=3)
    sobely = cv2.Sobel(image, cv2.CV_64F, 0, 1, ksize=3)

    # Calculate the variance
    sobel_var = np.var(sobelx) + np.var(sobely)
    return sobel_var


def sobel_xmp(file_path='./ImageJPG/image1.jpg'):
    # Use xmp_read from exifrw.py to read the xmp of all images
    xmp_data = exifrw.xmp_get_dict(file_path)
    if "SobelFilterResult" not in xmp_data:
        sovel_var = sobel_variance(file_path)
        # Write the Sobel variance to the XMP with the tag "SobelFilterResult"
        exifrw.xmp_write(file_path, "SobelFilterResult", str(sovel_var))
        # Display the filename currently being processed
        print(f"Processing {file_path}...")
        # Display the added tag
        print(exifrw.xmp_get_dict(file_path))
    else:
        pass
    return f"Processed {file_path} with Sobel Variance: {sovel_var}"


if __name__ == "__main__":
    sobel_xmp("./ImageJPG/image1.jpg")


"""
    import time

    start_time = time.time()
    for x in range(100):
        for i in range(1, 5):  # 1から5までの連番
            image_path = f'Camera-test\Evaluation-desert\iss{i}.jpg'  # 画像のパスを生成
            result = sobel_variance(image_path)

            if result is not None:
                print(f"Sobel Variance for the image at {image_path}: {result}")
        end_time = time.time()
        elapsed_time = end_time - start_time
        print(f"実行時間: {elapsed_time} 秒")
"""
