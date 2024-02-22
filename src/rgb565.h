// Read images from file system to display on screen

#ifndef _RGB565_H_
#define _RGB565_H_

#include <TFT_eSPI.h>

typedef uint16_t Pixel;

typedef struct ImageInfo {
  uint32_t x, y;
  uint32_t w, h;
  size_t buffSize;        // buff size in Pixels
  Pixel* pixelBuff;       // == nullptr to just get size info
} ImageInfo;

typedef enum ImageError {
  IE_OK = 0,
  IE_FILE_NOT_FOUND,
  IE_INVALID_FORMAT,
  IE_HDR_SIZE_MISMATCH,
  IE_BUFFER_TOO_SMALL,
  IE_POSITION_OFF_SCREEN
} ImageError;

// Read an image file from the file system
ImageError loadRGB565(const char* filename, ImageInfo& info);

// Read in an image file and display it on-screen at the given co-ordinates
ImageError drawRGB565(const char* filename, TFT_eSPI& tft, ImageInfo& info);

// Toggle the colon on and off
void drawOrClear(bool draw, TFT_eSPI& tft, ImageInfo& info);

#endif  // _RGB565_H_
