// Read images from file system to display on screen
#include "appInfo.h"
#include "rgb565.h"

#define IMAGE_BUFF_SIZE (DIGIT_W * DIGIT_H * sizeof(uint16_t))

// Fixed I/O buffer because it is too large to create on the stack
uint8_t digitBuff[IMAGE_BUFF_SIZE];
// Cache so don't need to keep re-loading it every other second
uint8_t colonBuff[COLON_W * DIGIT_H * sizeof(uint16_t)];
int16_t colonX, colonY;

uint16_t be16(fs::File &f)
{
  uint16_t result;
  
  ((uint8_t *)&result)[1] = f.read(); // MSB
  ((uint8_t *)&result)[0] = f.read(); // LSB
  return result;
}

uint32_t be32(fs::File &f)
{
  uint32_t result;
  
  ((uint8_t *)&result)[3] = f.read(); // MSB
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[0] = f.read(); // LSB
  return result;
}

// Read in an image file and display it on-screen at the given co-ordinates
bool drawRGB565(const char *filename, TFT_eSPI& tft, int16_t x, int16_t y)
{
  if ((x >= tft.width()) || (y >= tft.height()))
    return false;
  
  uint32_t startTime = millis();
  fs::File imgFile = LittleFS.open(filename);

  if (!imgFile) {
    Serial.print("File not found: "); Serial.println(filename);
    return false;
  }

  bool rc = false;
  uint32_t magic = be32(imgFile);
  uint32_t w = be32(imgFile);
  uint32_t h = be32(imgFile);

  if (magic == 0x52353635) {          // 'R565'
    PRTF("image is %dW x %dH\n", w, h);

    if (w * h * sizeof(uint16_t) <= IMAGE_BUFF_SIZE) {
      be32(imgFile);      // skip reserved bytes
      imgFile.read(digitBuff, w * h * sizeof(uint16_t));
      tft.pushImage(x, y, w, h, (uint16_t *)digitBuff);
      rc = true;

      PRT(filename); PRT(" loaded in "); PRT(millis() - startTime); PRTLN(" ms");
    }
    else {
      Serial.print(filename); Serial.print(" too large to load");
      // Serial.printf("size: %ld max: %ld\n", w * h * sizeof(uint16_t), IMAGE_BUFF_SIZE);
  }
  }
  else {
      Serial.print("File not RGB565 format: 'magic' = 0x"); Serial.println(magic, HEX);
  }

  imgFile.close();
  return rc;
}

// Load, display and cache the "colon on" image
void loadColon(const char* filename, TFT_eSPI& tft, int16_t x, int16_t y)
{
  colonX = x;
  colonY = y;
  drawRGB565(filename, tft, colonX, colonY);
  // Cache for repeated use later
  memcpy(colonBuff, digitBuff, sizeof(colonBuff));
}

// Toggle the colon on and off
void drawColon(bool on, TFT_eSPI& tft)
{
  if (on)   // use our cached image
  {
    tft.pushImage(colonX, colonY, COLON_W, DIGIT_H, (uint16_t *)colonBuff);
  }
  else      // just blank the area
  {
    tft.fillRect(colonX, colonY, COLON_W, DIGIT_H, TFT_BLACK);
  }
}
