#pragma once

#include <Arduino.h>

#define HOSTNAME "flip-clock"
#define MY_TZ_SERVER "us.pool.ntp.org"      // time server pool
#define MY_TZ "EST5EDT,M3.2.0,M11.1.0"      // America/New_York

// Display formats: 24-hour clock, 12-hour, and 12-hour with leading blank
enum DISP_FMT { HR24_FMT, HR12_FMT, HR12B_FMT, NUM_FMTS };
extern DISP_FMT dispFmt;
#define INITIAL_DISP_FMT HR12_FMT

// Demo mode indicator
extern bool runFast;

// Display digit indicies (left to right)
enum DIG_IDXS { HR10, H01, MIN10, MIN01, NUM_DIGITS };

// values based on generated digit images
#define DIGIT_W (64)
#define DIGIT_H (106)
#define COLON_W (27)
#define PAD_W (2)
#define CLOCK_W ((NUM_DIGITS * DIGIT_W) + COLON_W + (NUM_DIGITS * PAD_W))
#define MAX_STEP (4)    // # steps to "roll" from one digit to next

// Debugging short-cuts
#if 0
  #define PRT(x) Serial.print(x)
  #define PRTLN(x) Serial.println(x)
  #define PRTF(...) Serial.printf(__VA_ARGS__)
#else
  #define PRT(x)
  #define PRTLN(x)
  #define PRTF(...)
#endif
