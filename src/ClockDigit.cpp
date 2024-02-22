/*
  Encapsulate a single digit's display
*/
#include "ClockDigit.h"

// Set the list of available digit characters to be displayed.
// This is always 0-9 for "units" digits, but "tens" digits are
// usually restricted to fewer characters (0-1, 0-2, 0-5).
// 'x' is used to stand in for a blank to avoid filesystem issues.
void ClockDigit::setDigitList(const char* digits) {
  int nDigits = strlen(digits);

  digitSet = digits;
  if (currIdx >= nDigits || nextIdx >= nDigits) {
    // reset if currently out-of-range
    currIdx = nextIdx = step = 0;
  }
}

// Set the next digit to "flip" to, and the current digit if not stepping.
void ClockDigit::setDigit(int digit, bool mayStep) {
  nextIdx = digit % strlen(digitSet);
  if (!mayStep) currIdx = nextIdx;
  step = 0;
}

// Load the digit's image file and display on screen
void ClockDigit::draw(TFT_eSPI& tft) {
  fnBuff[1] = digitSet[currIdx];
  fnBuff[2] = digitSet[nextIdx];
  fnBuff[3] = '0' + step;
  drawRGB565(fnBuff, tft, imgInfo);
}

// Animate a step of the flip from the current to the next digit.
bool ClockDigit::doStep(TFT_eSPI& tft) {
  if (currIdx != nextIdx) {   // reason to step?
    step += 1;
    if (step >= MAX_STEP) {   // next digit reached
      currIdx = nextIdx;
      step = 0;
    }
    draw(tft);
    return true;
  }
  return false;   // no step needed
}
