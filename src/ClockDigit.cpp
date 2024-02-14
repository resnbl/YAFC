/*
  Contain and manipulation the digit character images on screen.
*/
#include "ClockDigit.h"
#include "rgb565.h"

ClockDigit hr10 = { "012" };            // '00' <= hour <= '23'
ClockDigit hr01 = { "0123456789" };     // hour units digit
ClockDigit min10 = { "012345" };        // minute tens digit
ClockDigit min01 = { "0123456789" };    // minute units digit
ClockDigit* theClock[] = { &hr10, &hr01, &min10, &min01 };

bool stepping = false;
char fnBuff[] = "/000.rgb565";

// Initialize digits' screen position
void setDigitPos(int *positions) {
  for (int i = HR10; i <= MIN01; ++i) {
    theClock[i]->posX = *positions++;
    theClock[i]->posY = *positions++;
  }
}

// Change clock display format
void setDisplayFormat(DISP_FMT fmt)
{
  if (fmt == HR24_FMT) hr10.digits = "012";
  else if (fmt == HR12_FMT) hr10.digits = "01";
  else hr10.digits = "x1";
}

// "Reset" clock to specified hours/minutes
void setDigits(int hours, int mins) {
  if (dispFmt != HR24_FMT) {
    if (hours > 12) hours -= 12;
    if (hours == 0) hours = 12;
  }
  hr10.currIdx = hr10.nextIdx = hours / 10;
  hr01.currIdx = hr01.nextIdx = hours % 10;
  min10.currIdx = min10.nextIdx = mins / 10;
  min01.currIdx = min01.nextIdx = mins % 10;
  hr10.step = hr01.step = 0;
  min10.step = min01.step = 0;

  stepping = false;
}

// Update display every minute
void updateDigits(int hours, int mins) {
  if (dispFmt != HR24_FMT) {
    if (hours > 12) hours -= 12;
    if (hours == 0) hours = 12;
  }
  hr10.nextIdx = hours / 10;
  hr01.nextIdx = hours % 10;
  min10.nextIdx = mins / 10;
  min01.nextIdx = mins % 10;
  hr10.step = hr01.step = 0;
  min10.step = min01.step = 0;

  stepping = true;
}

// True if digits are currently being "rolled"
bool areStepping() {
  return stepping;
}

// Load the appropriate digit image at its designated position:
void drawDigit(ClockDigit& dig, TFT_eSPI& tft) {
  fnBuff[1] = dig.digits[dig.currIdx];
  fnBuff[2] = dig.digits[dig.nextIdx];
  fnBuff[3] = '0' + dig.step;
  drawRGB565(fnBuff, tft, dig.posX, dig.posY);
}

// Re-draw screen
void drawAllDigits(TFT_eSPI& tft) {
  tft.fillScreen(TFT_BLACK);
  drawDigit(hr10, tft);
  drawDigit(hr01, tft);
  drawDigit(min10, tft);
  drawDigit(min01, tft);
}

/*
  Starting from the least significant digit to the most, check for a
  digit image that needs to be rolled on its change from one digit
  to the next. Disable stepping when all have been rolled.
  This implements a "cascade" effect of rolling digits (especially
  when time changes from 12:59 to 01:00).
*/
void doStep(TFT_eSPI& tft) {
  // from minutes units to hours tens...
  for (int dp = MIN01; dp >= HR10; --dp) { 
    if (theClock[dp]->currIdx != theClock[dp]->nextIdx) {
      // start or continue stepping
      if (++(theClock[dp]->step) >= MAX_STEP) {
        // end of stepping: set the full final digit image
        theClock[dp]->currIdx = theClock[dp]->nextIdx;
        theClock[dp]->step = 0;
      }
      drawDigit(*theClock[dp], tft);
      return;
    }
  }
  // nothing left to roll
  stepping = false;
}
