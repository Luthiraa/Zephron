import numpy as np
import cv2
import pyperclip
import os


def convert_img(file_name, img_name, width, height):
    """
    Convert an image to a C-style array (RGB565 format), copy to clipboard.

    :param file_name: Path to the input image.
    :param img_name: Name of the output C array.
    :param width: Target width of the image.
    :param height: Target height of the image.
    """
    if not os.path.exists(file_name):
        raise FileNotFoundError(f"File not found: {file_name}")

    img = cv2.imread(file_name)
    if img is None:
        raise ValueError(f"Failed to load image: {file_name}")

    img = cv2.resize(img, (width, height), interpolation=cv2.INTER_NEAREST)
    img = img.astype(np.uint16)

    # Convert BGR to RGB565 format
    img_r = img[:, :, 2] >> 3
    img_g = img[:, :, 1] >> 2
    img_b = img[:, :, 0] >> 3
    img = (img_r << 11) | (img_g << 5) | img_b

    img_flat = img.flatten()
    img_array = ', '.join(f"0x{x:04X}" for x in img_flat)
    output_code = f"unsigned short {img_name}[{width * height}] = {{{img_array}}};"

    # Add drawing functions
    output_code += "\n\n" + generate_draw_pixel_code(img_name, width, height)

    # Copy to clipboard
    pyperclip.copy(output_code)
    print("✅ C array and draw code copied to clipboard!")


def generate_draw_pixel_code(img_name, width, height):
    """
    Generate C code to draw and erase the image using plot_pixel().

    :param img_name: Name of the image array.
    :param width: Image width.
    :param height: Image height.
    :return: C function code as a string.
    """
    return f"""void plot_image_{img_name}(int x, int y) {{
    for (int i = 0; i < {height}; i++) {{
        for (int j = 0; j < {width}; j++) {{
            plot_pixel(x + j, y + i, {img_name}[i * {width} + j]);
        }}
    }}
}}

void erase_image_{img_name}(int x, int y) {{
    for (int i = 0; i < {height}; i++) {{
        for (int j = 0; j < {width}; j++) {{
            plot_pixel(x + j, y + i, 0);
        }}
    }}
}}"""


if __name__ == '__main__':
    convert_img(
        file_name='45deg.png',
        img_name='paimon',
        width=50,
        height=50
    )

