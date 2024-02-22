#ifndef _CLOCK_FACE_H_
#define _CLOCK_FACE_H_

#include <TFT_eSPI.h>
#include "flipclock.h"
#include "ClockDigit.h"

class ClockFace {
public:
  ClockFace(TFT_eSPI& the_tft);

  // Set the display format
  void setDisplayFormat(DISP_FMT fmt);      // setter
  DISP_FMT getDisplayFormat();              // getter
  bool isStepping() { return stepping; }    // getter (only)
  
  // Set the current hours:mins and if should initiate a "flip"
  void setTime(int hours, int mins, bool startStep);
  // Draw all digits directly
  void drawAll();
  // Step through a time increment
  void doStep();

  ClockDigit hr10 = ClockDigit("012");          // assumes HR24 format
  ClockDigit hr01 = ClockDigit("0123456789");
  ClockDigit min10 = ClockDigit("012345");
  ClockDigit min01 = ClockDigit("0123456789");

private:
  TFT_eSPI& tft;
  DISP_FMT dispFmt;
  bool stepping = false;
};

#endif    // _CLOCK_FACE_H_
