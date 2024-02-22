#include "ClockFace.h"

ClockFace::ClockFace(TFT_eSPI& the_tft) : tft(the_tft) {
  setDisplayFormat(INITIAL_DISP_FMT);
}

// Set the display format (only affects the hours "tens" digit)
void ClockFace::setDisplayFormat(DISP_FMT fmt) {
  if (fmt == HR24_FMT) {
    hr10.setDigitList("012");
  }
  else if (fmt == HR12_FMT) {
    hr10.setDigitList("01");
  }
  else if (fmt == HR12B_FMT) {
    hr10.setDigitList("x1");
  }

  dispFmt = fmt;
}

DISP_FMT ClockFace::getDisplayFormat() {
  return dispFmt;
}

// Set the current hours:mins and if should initiate a "flip"
void ClockFace::setTime(int hours, int mins, bool startStep) {
  if (dispFmt != HR24_FMT) {
    if (hours > 12) hours -= 12;
    else if (hours == 0) hours = 12;
  }
  stepping = startStep;
  hr10.setDigit(hours / 10, stepping);
  hr01.setDigit(hours % 10, stepping);
  min10.setDigit(mins / 10, stepping);
  min01.setDigit(mins % 10, stepping);
}

// Draw all digits directly (at init or after gross tiem change)
void ClockFace::drawAll() {
  hr10.draw(tft);
  hr01.draw(tft);
  min10.draw(tft);
  min01.draw(tft);
}

// Step through a time increment: mins than hours
void ClockFace::doStep() {
  if (stepping) {
    stepping = min01.doStep(tft) || min10.doStep(tft) ||
      hr01.doStep(tft) || hr10.doStep(tft);
  }
}
