// Read images from file system to display on screen
#include <LittleFS.h>
#include "flipclock.h"
#include "rgb565.h"


// Read the next 2 bytes from file as a big-endian 16-bit integer
uint16_t be16(fs::File &f)
{
  uint16_t result;
  
  ((uint8_t *)&result)[1] = f.read(); // MSB
  ((uint8_t *)&result)[0] = f.read(); // LSB
  return result;
}

// Read the next 4 bytes from file as a big-endian 32-bit integer
uint32_t be32(fs::File &f)
{
  uint32_t result;
  
  ((uint8_t *)&result)[3] = f.read(); // MSB
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[0] = f.read(); // LSB
  return result;
}

// Load an image (or just its info)
ImageError loadRGB565(const char* filename, ImageInfo& info) {
  fs::File imgFile;
  uint32_t magic;
  ImageError rc;

  if (imgFile = LittleFS.open(filename)) {
    magic = be32(imgFile);
    if (magic == 0x52353635) {      // 'R565'
      info.w = be32(imgFile);
      info.h = be32(imgFile);
      be32(imgFile);                // skip reserved bytes

      size_t numPixels = info.w * info.h;
      size_t numBytes = numPixels * sizeof(Pixel);
      size_t bytesRead;

      if (info.pixelBuff) {         // buffer provided?
        if (info.buffSize >= numPixels) {
          bytesRead = imgFile.read((uint8_t*)info.pixelBuff, numBytes);
          if (bytesRead == numBytes) {
            rc = IE_OK;
          }
          else
            rc = IE_HDR_SIZE_MISMATCH;
        }
        else
          rc = IE_BUFFER_TOO_SMALL;
      }
      else {    // just wanted size info
        info.buffSize = numPixels;
        rc = IE_OK;
      }
    }
    else
      rc = IE_INVALID_FORMAT;

    imgFile.close();
  }
  else
    rc = IE_FILE_NOT_FOUND;
    
  return rc;
}

// Read in an image file and display it on-screen at the given co-ordinates
ImageError drawRGB565(const char *filename, TFT_eSPI& tft, ImageInfo& info)
{
  ImageError rc;

  if ((info.x <= tft.width()) && (info.y <= tft.height())) {
    rc = loadRGB565(filename, info);
    if (rc == IE_OK) {
      tft.pushImage(info.x, info.y, info.w, info.h, info.pixelBuff);
    }
  }
  else {
    rc = IE_POSITION_OFF_SCREEN;
  }

  return rc;
}

// Draw image from buffer or just clear the area
void drawOrClear(bool draw, TFT_eSPI& tft, ImageInfo& info)
{
  if (draw)   // use our cached image
  {
    tft.pushImage(info.x, info.y, info.w, info.h, info.pixelBuff);
  }
  else        // just blank the area
  {
    tft.fillRect(info.x, info.y, info.w, info.h, TFT_BLACK);
  }
}
