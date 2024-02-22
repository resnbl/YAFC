#ifndef _CLOCK_DIGIT_H_
#define _CLOCK_DIGIT_H_

#include <TFT_eSPI.h>
#include "rgb565.h"

#define MAX_STEP 4

class ClockDigit {
public:
  ClockDigit() {};
  ClockDigit(const char* digits) : digitSet(digits) {}

  // Set the list of available digit characters to be displayed.
  void setDigitList(const char* digits);

  // Save the image size, position, etc.
  void saveInfo(ImageInfo& info) { imgInfo = info; }

  // Set the next digit to "flip" to
  void setDigit(int digit, bool mayStep);

  // Load the digit's image file and display on screen
  void draw(TFT_eSPI& tft);
  
  // Animate digit flip
  bool doStep(TFT_eSPI& tft);

  int currIdx = 0;    // index into digitset for current digit value
  int nextIdx = 0;    // index into digitset for next digit value when stepping
  int step = 0;       // "flip" step (0 - MAX_STEP-1)

private:
  const char* digitSet;
  ImageInfo imgInfo;
  char fnBuff[16] = "/xxx.rgb565";
};

#endif    // _CLOCK_DIGIT_H_
