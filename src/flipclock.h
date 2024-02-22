#ifndef _FLIPCLOCK_H_
#define _FLIPCLOCK_H_

#include <Arduino.h>

#define HOSTNAME "flip-clock"
#define MY_TZ_SERVER "us.pool.ntp.org"      // time server pool
#define MY_TZ "EST5EDT,M3.2.0,M11.1.0"      // America/New_York

// Display formats: 24-hour clock, 12-hour, and 12-hour with leading blank
typedef enum DISP_FMT { HR24_FMT, HR12_FMT, HR12B_FMT, NUM_FMTS } DISP_FMT;
#define INITIAL_DISP_FMT HR12_FMT

// ESP32 pin numbers for buttons
#define TOP_PIN 14
#define BOT_PIN 0

// Debugging short-cuts
#if 1
  #define PRT(x) Serial.print(x)
  #define PRTLN(x) Serial.println(x)
  #define PRTF(...) Serial.printf(__VA_ARGS__)
#else
  #define PRT(x)
  #define PRTLN(x)
  #define PRTF(...)
#endif

#endif  // _FLIPCLOCK_H_