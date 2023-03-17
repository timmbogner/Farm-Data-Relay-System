#include <sys/time.h>

// select Time, in minutes, between time printed configuration
#if defined(TIME_PRINTTIME)
#define FDRS_TIME_PRINTTIME TIME_PRINTTIME
#else
#define FDRS_TIME_PRINTTIME GLOBAL_TIME_PRINTTIME
#endif // TIME_PRINTTIME
#define DSTSTART    (timeinfo.tm_mon == 2 && timeinfo.tm_wday == 0 && timeinfo.tm_mday > 7 && timeinfo.tm_mday < 15 && timeinfo.tm_hour == 2)
#define DSTEND  (timeinfo.tm_mon == 10 && timeinfo.tm_wday == 0 && timeinfo.tm_mday < 8 && timeinfo.tm_hour == 2)


time_t now;                           // Current time - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
struct timeval tv;
char strftime_buf[64];
bool validTimeFlag = false;           // Indicate whether we have reliable time 
time_t lastNTPFetchSuccess = 0;      // Last time that a successful NTP fetch was made
bool isDST;                           // Keeps track of Daylight Savings Time vs Standard Time
long slewSecs = 0;                  // When time is set this is the number of seconds the time changes
time_t lastUpdate = 0;
time_t lastTimeSend = 0;

void sendTimeLoRa();
esp_err_t sendTimeESPNow();

bool validTime() {
  if(now < 1677000000 || (millis() - lastNTPFetchSuccess > (24*60*60*1000))) {
    if(validTimeFlag) {
      DBG("Time no longer reliable.");
      validTimeFlag = false;
    }
    return false;
  }
  else {
    if(!validTimeFlag) {
      validTimeFlag = true;
    }
    return true;
  }
}

void printTime() {
  if(!validTime()) {
    return;
  }

  // UTC Time
  // now -= localOffset;
  // // print Unix time:
  // //DBG("Unix time = " + String(now));
  // localtime_r(&now, &timeinfo);
  // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  // DBG("The current UTC date/time is: " + String(strftime_buf));
  // now += localOffset;

  // Local time
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  DBG("The current local date/time is: " + String(strftime_buf));
}

void checkDST() {
  // DST -> STD - subtract one hour (3600 seconds)
  if(validTimeFlag && isDST && (DSTEND || timeinfo.tm_isdst == 1)) {
    isDST = false;
    now -= 3600;
    localtime_r(&now, &timeinfo); // write to timeinfo struct
    DBG("Time change from DST -> STD");
  }
  // STD -> DST - add one hour (3600 seconds)
  else if(validTimeFlag && !isDST && (DSTSTART || timeinfo.tm_isdst == 0)) {
    isDST = true;
    now += 3600;
    localtime_r(&now, &timeinfo); // write to timeinfo struct
    DBG("Time change from STD -> DST");
  }
  return;
}

bool setTime(time_t previousTime) {
  slewSecs = 0;

  if(previousTime != 0) {
    slewSecs = now - previousTime;
    DBG("Time adjust " + String(slewSecs) + " secs");
  }

  // time(&now);
  localtime_r(&now, &timeinfo); // write to timeinfo struct
  mktime(&timeinfo); // set tm_isdst flag
  // Check for DST/STD time and adjust accordingly
  checkDST();
  tv.tv_sec = now;
  settimeofday(&tv,NULL); // set the RTC time
  // Uncomment below to send time and slew rate to the MQTT server
  // loadFDRS(now, TIME_T, 111);
  // loadFDRS(slewSecs, TIME_T, 111);
  // Do not call sendFDRS here.  It will not work for some reason.
  if(validTime()) {
    lastNTPFetchSuccess = millis();
    printTime();
    return true;
  }
  else {
    return false;
  }
}

// Periodically send time to ESP-NOW or LoRa nodes associated with this gateway/controller
void sendTime() {
  if(validTime()) { // Only send time if it is valid
  DBG("Sending out time");
  // Only send via Serial interface if WiFi is enabled to prevent loops
#ifdef USE_WIFI // do not remove this line
    sendTimeSerial();
#endif          // do not remove this line
    sendTimeLoRa();
    sendTimeESPNow();
  }
}

void updateTime() {

  if(millis() - lastUpdate > 500) {
    time(&now);
    localtime_r(&now, &timeinfo);
    tv.tv_sec = now;
    tv.tv_usec = 0;
    validTime();
    checkDST();
    lastUpdate = millis();
  }
  if(validTimeFlag && (millis() - lastTimeSend) > (1000 * 60 * TIME_SEND_INTERVAL)) {
    sendTime();
    lastTimeSend = millis();
  }
}