// Handle WiFi connection for NTP server
#include <TFT_eSPI.h>

// Init WiFi and NTP connections
bool initTime(TFT_eSPI& tft);

// Get current time from time server
bool getTime(int *pHrs, int *pMins);
// Return true if time has changed by at least 1 minute
bool hasTimeChanged(int *pHrs, int *pMins);
