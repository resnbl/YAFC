/*
  Yet Another Flip Clock app for LilyGo T-Display S3
*/
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <OneButton.h>
#include "flipclock.h"
#include "ClockFace.h"
#include "rgb565.h"
#include "networking.h"

#define SCR_FONT_NUM (2)    // TFT_eSPI font #
#define SCR_FONT_H (16)     // its height
#define DIGIT_PAD (2)       // space aroud digits
#define STEP_MS 100         // duration of each flip step
#define COLON_MS 1000       // duration of colon ON | OFF
#define BABBLE_MS 4000      // duration to leave info msgs on screen

TFT_eSPI tft = TFT_eSPI();
ClockFace theClock = ClockFace(tft);
OneButton fmtButton(TOP_PIN);
OneButton demoButton(BOT_PIN);

ImageInfo digitInfo, colonInfo;
int scrW, scrH;
int hours, mins;
int tickMS;
bool runFast = false;
bool colonOn = true;
bool babbleOn = false;
unsigned long tickStart, colonStart, stepStart, babbleStart;
const char* dispFmtLabels[] = { "24-hour", "12-hour", "12-hour leading blank" };


// Initialize TFT screen
void screenSetup() {
  tft.begin();
  tft.setRotation(1);                 // 0 & 2 Portrait. 1 & 3 landscape
  tft.setSwapBytes(false);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, 0, SCR_FONT_NUM);  // font #2 = 16px text
  tft.setTextSize(1);                 // font multiplier = 1
  tft.println("Initializing...");
  scrW = tft.width();
  scrH = tft.height();
}

// Allocate file buffers for 1 digit and the colon image
void getImageInfos() {
  ImageError err = loadRGB565("/000.rgb565", digitInfo);

  if (!err) {
    digitInfo.pixelBuff = new Pixel [digitInfo.buffSize];

    err = loadRGB565("/colon1.rgb565", colonInfo);
    if (!err) {
      colonInfo.pixelBuff = new Pixel [colonInfo.buffSize];
      // might as well actually load it now
      err = loadRGB565("/colon1.rgb565", colonInfo);
    }
  }

  if (err) {
    tft.print("Error getting image info: ");
    tft.println(err);
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
}

// Set/copy the individual digit positions and shared file buffer
void setDigitsInfos() {
  int clockW = (4 * digitInfo.w) + colonInfo.w + (4 * DIGIT_PAD);

  // center digits & colon on display
  digitInfo.x = (scrW - clockW) / 2;
  digitInfo.y = (scrH - digitInfo.h) / 2;   // all are same height
  colonInfo.y = digitInfo.y;

  theClock.hr10.saveInfo(digitInfo);
  digitInfo.x += digitInfo.w + DIGIT_PAD;

  theClock.hr01.saveInfo(digitInfo);
  digitInfo.x += digitInfo.w + DIGIT_PAD;

  colonInfo.x = digitInfo.x;
  digitInfo.x += colonInfo.w + DIGIT_PAD;

  theClock.min10.saveInfo(digitInfo);
  digitInfo.x += digitInfo.w + DIGIT_PAD;

  theClock.min01.saveInfo(digitInfo);
}

// Draw short texts at bottom of screen
void babble(const char* txt) {
  if (txt) {
    tft.fillRect(0, scrH-SCR_FONT_H, scrW, scrH, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(txt, scrW / 2, scrH-(SCR_FONT_H / 2));
    babbleOn = true;
    babbleStart = millis();
  }
  else {
    // no text -> erase screen area
    tft.fillRect(0, scrH-SCR_FONT_H, scrW, scrH, TFT_BLACK);
    babbleOn = false;
  }
}

// Button handlers
void changeFormat() {
  DISP_FMT fmt = theClock.getDisplayFormat();
  fmt = (DISP_FMT)(((int)fmt + 1) % NUM_FMTS);
  theClock.setDisplayFormat(fmt);
  theClock.setTime(hours, mins, false);
  theClock.drawAll();
  babble(dispFmtLabels[fmt]);
}
void toggleDemo() {
  runFast = !runFast;
  tickMS = getTime(&hours, &mins, runFast, theClock.getDisplayFormat());
  tickStart = millis();
  theClock.setTime(hours, mins, false);
  theClock.drawAll();
  babble(runFast ? "Demo mode ON" : "Demo mode OFF");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  screenSetup();

  if (!LittleFS.begin(false)) {
    tft.println("LittleFS init failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  tft.println("LittleFS found");

  fmtButton.attachClick(changeFormat);
  demoButton.attachClick(toggleDemo);

  initTime(tft);
  getImageInfos();
  setDigitsInfos();
  delay(4000);
  
  tickMS = getTime(&hours, &mins, runFast, theClock.getDisplayFormat());
  theClock.setTime(hours, mins, false);
  tft.fillScreen(TFT_BLACK);
  theClock.drawAll();
  drawOrClear(colonOn, tft, colonInfo);
  tickStart = colonStart = millis();
}

void loop() {
  auto now = millis();

  if (theClock.isStepping() && (now - stepStart >= STEP_MS)) {
    theClock.doStep();
    stepStart = now;
  }
  else if (now - tickStart >= tickMS) {
    tickMS = updateTime(&hours, &mins, runFast);
    if (tickMS >= 0) {
      // simple increment
      theClock.setTime(hours, mins, true);
      theClock.doStep();
      stepStart = now;
    }
    else {
      // major change: reset display
      theClock.setTime(hours, mins, false);
      theClock.drawAll();
      babble("time re-sync");
      tickMS = -tickMS;
    }
    tickStart = now;
  }

  if (now - colonStart >= COLON_MS) {
    colonOn = !colonOn;
    drawOrClear(colonOn, tft, colonInfo);
    colonStart = now;
  }

  if (babbleOn && (now - babbleStart >= BABBLE_MS)) {
    babble(nullptr);
  }

  fmtButton.tick();
  demoButton.tick();
}
