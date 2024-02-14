/*
  We only connect to the network so we can get the current time.
*/
#include <Arduino.h>
#include <WiFi.h>
#include <ctime>
#include "appInfo.h"
#include "networking.h"
#include "secrets.h"

#define FAST_SEC_MS 6000      // # ms/min in "fast mode"

struct tm theTime;
unsigned long lastCheck;

// Initialize our network connection, then fetch the current time
bool initTime(TFT_eSPI& tft) {
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
  
  if (getLocalTime(&theTime)) {
    tft.println("Time synced");
    return true;
  }
  return false;
}

// Fetch the current time at start-up
bool getTime(int *pHrs, int *pMins) {
  if (runFast) {
    // override current time for demo purposes
    theTime.tm_hour = (dispFmt == HR24_FMT) ? 23 : 12;
    theTime.tm_min = 57;
    theTime.tm_sec = 0;
  }
  else {
    getLocalTime(&theTime);
  }

  *pMins = theTime.tm_min;
  *pHrs = theTime.tm_hour;

  lastCheck = millis();
  return true;
}

// Return true if time has changed by at least 1 minute
bool hasTimeChanged(int *pHrs, int *pMins) {
  if (millis() - lastCheck >= FAST_SEC_MS) {
    lastCheck = millis();

    if (runFast) {
      // always increment minutes in "fast mode"
      if (++theTime.tm_min >= 60) {
        theTime.tm_min = 0;
        if (++theTime.tm_hour >= 24)
          theTime.tm_hour = 0;
      }
      *pHrs = theTime.tm_hour;
      *pMins = theTime.tm_min;
      return true;
    }
    else {
      // check the "real" time
      getLocalTime(&theTime);
      if (*pMins != theTime.tm_min || *pHrs != theTime.tm_hour) {
        *pHrs = theTime.tm_hour;
        *pMins = theTime.tm_min;
        return true;
      }
      else {
        return false;
      }
    }
  }
  else {     // "too soon" to check
    return false;
  }
}
