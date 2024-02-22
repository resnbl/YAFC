"""
Generate images of digits for use in a "flip clock".
This includes whole digit images as well as combined "previous" digit" and
"new digit" to show a limited animation.
Output is in .png | .jpg | .bmp format for use in desktop programs
or in a custom RGB565 format for use with a particular TFT display.
"""
import PIL
from PIL import Image, ImageDraw, ImageFont
import pathlib
import struct

# Fill in these values:
FONT_FACE = "Roboto-Regular.ttf"            # desired (loaded in OS) font path
SCREEN_W, SCREEN_H = 320, 170               # screen width, height
PAD_LR, PAD_TB = 20, 20                     # padding
RECT_RADIUS = 10                            # rounded rectangle corner radius
SCREEN_BG = "black"                         # screen background (black)
DIGIT_BG = "#696969"                        # digits background (dim gray?)
DIGIT_FG = "white"                          # digits text color
DIGIT_SEP = "black"                         # "fold" line
FOLD_WIDTH = 4                              # fold line width
# OUT_PATH = 'data/'                          # output folder
# OUT_FORM = 'rgb565'                         # rgb565 | png | bmp | ...
OUT_PATH = 'pngs/'                          # output folder
OUT_FORM = 'png'                            # rgb565 | png | bmp | ...
BLANK_FLAG = 'x'                            # safe replacement for ' ' in filenames


# values to be computed
font_size = 0
digit_w = 0
digit_h = 0


def compute_sizes() -> tuple[int, int, int]:
    """ Calculate largest font size that will fit on display, and the width and height
        of a resulting single digit image."""
    max_text = "12:34"
    digit_text = "0"
    max_w = SCREEN_W - (2 * PAD_LR)
    max_h = SCREEN_H - (2 * PAD_TB)
    fsize = max_h      # initial guess too big!

    # Compute largest font size that fits on screen (with padding)
    while True:
        font = ImageFont.truetype(FONT_FACE, fsize)
        left, top, right, bottom = font.getbbox(max_text)
        if (right - left < max_w) and (bottom - top < max_h):
            break
        else:
            fsize -= 2

    # Compute single digit box size
    left, top, right, bottom = font.getbbox(digit_text)

    # Round digit sizes up if odd
    return fsize, right + (right & 0x01), bottom + (bottom & 0x01)


def convertRGB565(image: Image, outfile_path: pathlib.Path):
    """ Output an image in a custom format optimized for an ESP32 micro-controller display.
        00: 'R565' - "magic bytes"
        04: image width
        08: image height
        0C: reserved
        10: (width * height) 16-bit pixel colors as RED 5, GREEN 6, and BLUE 5 bits.
        Note: all values are in "big-endian" format to optimize display processing (even though
        most micro-controllers are natively "little-endian").
    """
    try:
        with open(outfile_path, 'wb') as output:
            hdr = struct.pack('>4s3I', b'R565', image.width, image.height, 0)   # big-endian w&h + 1 reserved int32s
            output.write(hdr)
            for pixel in image.getdata():
                int16 = ((pixel[0] & 0xF8) << 8) | ((pixel[1] & 0xFC) << 3) | ((pixel[2] & 0xF8) >> 3)
                output.write(struct.pack('>H', int16))

    except (FileNotFoundError, PIL.UnidentifiedImageError) as err:
        print(err)
        exit(1)


def saveImage(image: Image, filename: str):
    """Save image in desired format"""
    if OUT_FORM == 'rgb565':
        convertRGB565(image, pathlib.Path(f'{OUT_PATH}/{filename}.rgb565'))
    else:
        image.save(pathlib.Path(f'{OUT_PATH}/{filename}.{OUT_FORM}'))


def make_colons():
    """ Create (narrower) colon image to align with digits without digit background or divider """
    font = ImageFont.truetype(FONT_FACE, font_size)
    w = int(font.getlength(':'))    # use "real" colon width
    img = Image.new(mode="RGB", size=(w, digit_h), color=SCREEN_BG)
    saveImage(img, 'colon0')        # colon off = blank

    # draw two circles evenly positioned above/below center line
    drw = ImageDraw.Draw(img)
    dot_r = w // 4
    lr_center = w // 2
    tb_upper = digit_h // 4             # 1/4 from top
    x0, y0 = lr_center - dot_r, tb_upper - dot_r
    x1, y1 = lr_center + dot_r, tb_upper + dot_r
    drw.ellipse(((x0, y0), (x1, y1)), fill=DIGIT_FG)
    tb_lower = digit_h - tb_upper       # 1/4 from bottom
    x0, y0 = lr_center - dot_r, tb_lower - dot_r
    x1, y1 = lr_center + dot_r, tb_lower + dot_r
    drw.ellipse(((x0, y0), (x1, y1)), fill=DIGIT_FG)
    saveImage(img, 'colon1')        # colon on


def make_digit_image(digit: str, font: ImageFont) -> Image:
    """ Create an image of a single digit on a rounded rectangle background with a
        divider line across the middle to indicate where it "folds".
    """
    img = Image.new("RGB", (digit_w, digit_h), color=SCREEN_BG)
    drw = ImageDraw.Draw(img)
    drw.rounded_rectangle(((0, 0), (digit_w, digit_h)), radius=RECT_RADIUS, fill=DIGIT_BG)
    mid_h = digit_h // 2
    # digit 'x' is FS-safe name for 'blank' image used instead of leading 0's
    txt = ' ' if digit == BLANK_FLAG else digit[0]
    drw.text((0, mid_h), txt, font=font, fill=DIGIT_FG, anchor="lm")
    drw.line([(0, mid_h), (digit_w, mid_h)], fill=DIGIT_SEP, width=FOLD_WIDTH)
    return img


def make_images(text='01'):
    """ Create and save "whole" digit image as well as additional "steps" as the images
        "roll" from the initial digit to the final one.
    """
    font = ImageFont.truetype(FONT_FACE, font_size)
    init_dig = text[0]
    final_dig = text[1]

    # make image for initial/final digits
    init_img = make_digit_image(init_dig, font)
    final_img = make_digit_image(final_dig, font)

    # Create the "middle" step of change with the top half of final digit and bottom half of initial
    step2 = init_img.copy()
    top2 = final_img.crop((0, 0, digit_w, digit_h//2))
    step2.paste(top2, (0, 0))

    # Create first step of change:
    step1 = step2.copy()
    # Shrink top half of initial digit to half-size vertically
    init_top_tilt = init_img.resize((digit_w, digit_h//4), box=(0, 0, digit_w, digit_h//2))
    # Display it 1/4 - 1/2 way down our split image
    step1.paste(init_top_tilt, (0, digit_h//4))
    # Small line across top to add "thickness" to flipping card
    drw = ImageDraw.Draw(step1)
    drw.line((RECT_RADIUS, digit_h//4, digit_w - RECT_RADIUS, digit_h//4), fill=DIGIT_SEP, width=1)

    # Create third step of change:
    step3 = step2.copy()
    # Shrink bottom half of final digit to half-size vertically
    final_bottom_tilt = final_img.resize((digit_w, digit_h//4), box=(0, digit_h//2, digit_w, digit_h))
    # Display it 1/2 - 3/4 way down our split image
    step3.paste(final_bottom_tilt, (0, digit_h//2))
    # Small line across top for some depth
    drw = ImageDraw.Draw(step3)
    drw.line((RECT_RADIUS, (digit_h*3)//4, digit_w - RECT_RADIUS, (digit_h*3)//4), fill=DIGIT_SEP, width=1)

    # Save files in format: <initial_digit><final_digit><step>.ext
    saveImage(init_img, f'{init_dig}{init_dig}0')
    saveImage(step1, f'{init_dig}{final_dig}1')
    saveImage(step2, f'{init_dig}{final_dig}2')
    saveImage(step3, f'{init_dig}{final_dig}3')
    # saveImage(final_img, f'{final_dig}{final_dig}0')    # "final" image will be saved on next call


# main program:
font_size, digit_w, digit_h = compute_sizes()

print(f'Font size: {font_size}, digit w, h: {digit_w}, {digit_h}')

# Units transitions
digits = '01234567890'      # wrap around
for i in range(len(digits)-1):
    make_images(digits[i:i+2])

# Minutes tens wrap (59 -> 00)
make_images('50')

# Hours units wrap for 12-hour clock (12 -> 01)
make_images('21')
# Hours units wrap for 24-hour clock ('23' -> '00')
make_images('30')

# Hours tens wrap with leading zero (12 -> 01)
make_images('10')
# Hours tens wrap for 24-hour clock ('23' -> '00')
make_images('20')
# Hours tens wrap with leading blank (' 9' -> '10' & '12' -> ' 1')
make_images('x1')
make_images('1x')

make_colons()
