/*
  Contain and manipulation the digit character images on screen.
*/
#include <TFT_eSPI.h>
#include "appInfo.h"

typedef struct {
  const char* digits;     // allowed chars used in filenames
  int posX, posY;         // screen position
  int currIdx, nextIdx;   // indicies into digits
  int step;               // > 0 => rolling step
} ClockDigit;

// Initialize digits' screen position w/ int[NUM_DIGITS * 2]
void setDigitPos(int *positions);

// Change time display format
void setDisplayFormat(DISP_FMT fmt);

// "Reset" clock to specified hours/minutes
void setDigits(int hours, int mins);

// Update display every minute
void updateDigits(int hours, int mins);

// True if digits are currently being "rolled"
bool areStepping();

// Initial display of digits
void drawAllDigits(TFT_eSPI& tft);

// Roll next digit one step
void doStep(TFT_eSPI& tft);
