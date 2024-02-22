/*
  We only connect to the network so we can get the current time.
*/
#include <Arduino.h>
#include <WiFi.h>
#include <ctime>
#include "flipclock.h"
#include "networking.h"
#include "secrets.h"

#define FAST_MSECS 6000   // 6 secs per min in "demo" mode

struct tm theTime;

// Initialize our network connection, then fetch the current time
void initTime(TFT_eSPI& tft) {
  // config NTP *before* starting WiFi
  configTime(0, 0, MY_TZ_SERVER);     // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);             // Set environment variable with your time zone
  tzset();

  WiFi.setHostname(HOSTNAME);         // must call before .mode()
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSWD);
  tft.print("Connecting: ");

  while (WiFi.status() != WL_CONNECTED) {
    tft.print('.');
    delay(100);
  }
  tft.print("\nConnected as: "); tft.println(WiFi.localIP());
  
  tft.print("Fetching time: ");
  while (!getLocalTime(&theTime, 1000)) {
    tft.print('.');
    delay(100);
  }
  tft.printf("\nTime synced to: %02d:%02d:%02d\n", theTime.tm_hour,
    theTime.tm_min, theTime.tm_sec);
}

// Get current time from time server
int getTime(int *pHrs, int *pMins, bool runFast, DISP_FMT dispFmt) {
  int waitMS;     // how long to wait before checking again

  if (runFast) {
    // override current time for demo purposes
    theTime.tm_hour = (dispFmt == HR24_FMT) ? 23 : 12;
    theTime.tm_min = 57;
    theTime.tm_sec = 0;
    waitMS = FAST_MSECS;
  }
  else {
    getLocalTime(&theTime);
    waitMS = (60 - theTime.tm_sec) * 1000;    // wait until end of minute
  }

  *pMins = theTime.tm_min;
  *pHrs = theTime.tm_hour;
  PRTF("getTime: %02d:%02d, wait %d\n", theTime.tm_hour, theTime.tm_min, waitMS);
  return waitMS;
}

// Increment or reset current time & return msecs until next change
int updateTime(int *pHrs, int *pMins, bool runFast) {
  int waitMS;     // how long to wait before checking again

  if (runFast) {
    // always increment minutes in "fast mode"
    if (++theTime.tm_min >= 60) {
      theTime.tm_min = 0;
      if (++theTime.tm_hour >= 24)
        theTime.tm_hour = 0;
    }
    waitMS = FAST_MSECS;
  }
  else {
    // check the "real" time
    getLocalTime(&theTime);

    // Since we only have digit transformations for one unit at a time,
    // ensure that we're only incrementing by 1 minute,
    // else we need to reset the display completely.
    int prevMins = *pHrs * 60 + *pMins;
    int currMins = theTime.tm_hour * 60 + theTime.tm_min;
    int minsDiff = currMins - prevMins;
    
    // positive wait if incr by 1 min (including wrap from 23:59 to 00:00) else negative
    waitMS = (60 - theTime.tm_sec) * 1000;    // time until next change
    if ((minsDiff != 1) && (minsDiff != -(23 * 60 + 59)))
      waitMS = -waitMS;
  }

  // store back new time
  *pHrs = theTime.tm_hour;
  *pMins = theTime.tm_min;
  PRTF("updateTime: %02d:%02d, wait %d\n", theTime.tm_hour, theTime.tm_min, waitMS);
  return waitMS;
}
