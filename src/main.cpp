/*
  Yet Another Flip Clock app for LilyGo T-Display S3
*/
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <OneButton.h>
#include "rgb565.h"
#include "appInfo.h"
#include "ClockDigit.h"
#include "networking.h"

#define STEP_MS 100
#define COLON_MS 1000
#define BABBLE_MS 4000
#define TOP_PIN 14
#define BOT_PIN 0

TFT_eSPI tft = TFT_eSPI();
OneButton fmtButton(TOP_PIN);
OneButton demoButton(BOT_PIN);

int scrW, scrH;
int hours, mins;
DISP_FMT dispFmt = INITIAL_DISP_FMT;
bool runFast = false;
bool colonOn = false;
bool babbleOn = false;
unsigned long colonStart, stepStart, babbleStart;
const char* disp_fmt_labels[] = { "24-hour", "12-hour", "12-hour leading blank" };

// Draw short texts at bottom of screen
void babble(const char* txt) {
  tft.fillRect(0, scrH-16, scrW, scrH, TFT_BLACK);
  // tft.setCursor(0, scrH-16, 2);
  // tft.print(txt);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(txt, scrW / 2, scrH-(16 / 2));
  babbleOn = true;
  babbleStart = millis();
}
void clearBabble() {
  tft.fillRect(0, scrH-16, scrW, scrH, TFT_BLACK);
  babbleOn = false;
}

// Button handlers
void fmtClick() {
  dispFmt = (DISP_FMT)(((int)dispFmt + 1) % NUM_FMTS);
  setDisplayFormat(dispFmt);
  setDigits(hours, mins);
  drawAllDigits(tft);
  babble(disp_fmt_labels[dispFmt]);
}

void demoClick() {
  runFast = !runFast;
  getTime(&hours, &mins);
  setDigits(hours, mins);
  drawAllDigits(tft);
  babble(runFast ? "Demo mode ON" : "Demo mode OFF");
}

void setup(){
  Serial.begin(115200);
  delay(2000);

  fmtButton.attachClick(fmtClick);
  demoButton.attachClick(demoClick);

  tft.begin();
  tft.setRotation(1);     // 0 & 2 Portrait. 1 & 3 landscape
  tft.setSwapBytes(false);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 0, 2);     // font #2 = 16px text
  tft.setTextSize(1);         // font multiplier = 1
  tft.println("Initializing...");

  scrW = tft.width();
  scrH = tft.height();

  if (!LittleFS.begin(false)) {
    tft.println("LittleFS init failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  tft.println("LittleFS found");

  if (!initTime(tft)) {
    tft.println("Time init failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  
  if (!getTime(&hours, &mins)) {
    tft.println("Cannot fetch time!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  delay(5000);
  
  // compute screen positions of images
  int x = (scrW - CLOCK_W) / 2;
  int y = (scrH - DIGIT_H) / 2;
  int positions[NUM_DIGITS * 2];
  int *p = positions;
  *p++ = x;   // hour tens
  *p++ = y;
  x += DIGIT_W + PAD_W;
  *p++ = x;   // hour units
  *p++ = y;
  x += DIGIT_W + PAD_W;

  loadColon("/colon1.rgb565", tft, x, y);
  x += COLON_W + PAD_W;

  *p++ = x;   // minute tens
  *p++ = y;
  x += DIGIT_W + PAD_W;
  *p++ = x;   // minute units
  *p++ = y;
  setDigitPos(positions);

  setDisplayFormat(dispFmt);
  setDigits(hours, mins);
  drawAllDigits(tft);

  colonStart = millis();
}

void loop() {
  auto now = millis();

  if (areStepping() && (now - stepStart >= STEP_MS)) {
    doStep(tft);
    stepStart = now;
  }
  else {
    int prevMins = hours * 60 + mins;
    if (hasTimeChanged(&hours, &mins)) {
      if (abs(hours * 60 + mins - prevMins) > 1) {
        // DST change or catch-up with real time
        setDigits(hours, mins);
        drawAllDigits(tft);
        babble("re-sync time");
      }
      else {
        updateDigits(hours, mins);
        doStep(tft);
        stepStart = now;
      }
    }
  }

  if (now - colonStart >= COLON_MS) {
    colonOn = !colonOn;
    drawColon(colonOn, tft);
    colonStart = now;
  }

  if (babbleOn && (now - babbleStart >= BABBLE_MS))
    clearBabble();

  fmtButton.tick();
  demoButton.tick();
}
