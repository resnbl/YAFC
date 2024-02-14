# YAFC: Yet Another Flip-clock

It took some work to get the `factory` program included with the **LilyGo T-Display S3** sample code to get running without the need for their specialized phone app, but I made it work. Imagine my disappointment when I found their flip clock did not actually "flip" - it just re-painted one digit on top of another.

So I decided to write my own version of a flip clock with some simple animation of the digits rolling over:

<p style="text-align: center">
<img src="doc-images/000.png">
<img src="doc-images/990.png">
<img src="doc-images/colon1.png">
<img src="doc-images/550.png">
<img src="doc-images/451.png">...
<img src="doc-images/452.png">
<img src="doc-images/453.png">
<img src="doc-images/550.png">
</p>

I wrote a Python program (detailed below) to generate the images I needed. This consisted of each whole digit, plus a series of three additional images with the first digit tilted at 45, 90 and 135 degrees overlaid on the second digit being "flipped" to. These images get stored on the device's flash memory using the `LittleFS` file system.

At run-time, pressing the top/right-of-USB button will toggle the display between 24-hour format, 12-hour format, and 12-hour with a leading blank insted of '0'. Pressing the bottom/left-of-USB button will toggle "demo mode" on and off. When "on", the time is set to a few minutes before a full 4-digit cascade of digits will occur and runs at 6 seconds per minute instead of the normal 60 seconds per minute.

## Setup

First, either rename or make a copy of the `my-secrets.h` file as `secrets.h` and insert your WiFi credentials in there.

The `appInfo.h` file contains definitions you may wish to change, including your host name and time zone. You can also choose the intial time display format with the `INITIAL_DISP_FMT` definition.

## Modules

Below are brief descriptions of the code. Additional information is in the comments in each.

### main

In `main.cpp`, the `setup()` function initializes the WiFi and time service as well as the display. `loop()` checks for and then over-writes a single digit for the "cascading, rolling" effect if needed. Otherwise, it checks for a change to the time (hours and minutes) and will kick off the rolling animation for, at a minimum, the minute units digit. It also flashes the colon image on and off.

### networking

The module initiates the WiFi connection and NTP time service to fetch the initial time.

### rgb565

This module deals with reading image files from disk and displaying them using the `TFT_eSPI` library. The image for the flashing colon is cached to avoid re-reading it many times per minute. (When the colon is off, the area is simply filled with a black rectangle.)

Images are stored in a format optimzed for this display. Each pixel is stored as a 16-bit value with 5 bits for RED, 6 bits for GREEN and 5 bits for BLUE. They are provided in "big-endian" order so they can be copied directly into display buffers (even though the ESP32-S3 is a "little endian" machine).

### ClockDigit

This code holds descriptors for each screen digit image and the routines needed to initialize and change them. Each digit is given a string listing the possible characters for that digit position. (The units positions always use all 10 digits, but the tens positions have more restricted sets of digits to display.)

## Image generation (flip_digits.py)

This Python program uses the Pillow (PIL) library to generate image files that are written to the `data` directory. It can generate the files in standard formats such as PNG, BMP, etc., but I use it here for a custom RGB565 format. This file format contains:

| offset | length | use                 |
| -----: | -----: | ------------------- |
|      0 |      4 | magic 'R565' flag   |
|      4 |      4 | image width in pixels |
|      8 |      4 | image height in pixels |
|     12 |      4 | reserved            |
|     16 |      * | width * length 16-bit RGB565 values |

All values are in "big-endian" format (although ESP32's are natively
"little-endian") because that is the most efficient way to load the display buffer.

Files all have 3-character names: The first char denotes the "main" digit being displayed, the second char is the digit being rolled to and the third is the step number 0-3. For example, the '0' digit file is named '000', the step files when rolling are named '011', '012', '013' and the final file in the sequence is '110' (as shown above).

Although this format has a certain advantage in loading efficiency and is 1/3 smaller than a `.bmp` file, the processing savings are not as big as I originally imagined - the bulk of image loading time is spent in reading from the `LittleFS` file system which can vary by 100% simply based on where the file is located in the flash, and even the fastest reads are still 5 times longer than the time needed to actually process the data, whatever format it is in. Had I known this before, I would probably have stuck with the `.bmp` format as those files can be processed by manu other programs.

Items you may wish to change in `flip_digits.py`:

  - FONT_FACE: choose a font you have on your desktop.
  - SCREEN_W and SCREEN_H: if using a different-sized display.
  - *_FG and *_BG: colors for the digits.
