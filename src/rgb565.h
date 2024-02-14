// Read images from file system to display on screen
#include <LittleFS.h>
#include <TFT_eSPI.h>

// Read in an image file and display it on-screen at the given co-ordinates
bool drawRGB565(const char* filename, TFT_eSPI& tft, int16_t x, int16_t y);

// Load, display and cache the "colon on" image
void loadColon(const char* filename, TFT_eSPI& tft, int16_t x, int16_t y);

// Toggle the colon on and off
void drawColon(bool on, TFT_eSPI& tft);
