// Handle WiFi connection for NTP server
#include <TFT_eSPI.h>

// Init WiFi and NTP connections
void initTime(TFT_eSPI& tft);

// Get current time from time server
int getTime(int *pHrs, int *pMins, bool runFast, DISP_FMT dispFmt);

// Increment or reset current time & return msecs until next change
int updateTime(int *pHrs, int *pMins, bool runFast);
