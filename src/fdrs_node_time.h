#include <sys/time.h>

// select Time, in minutes, between time printed configuration
#if defined(TIME_PRINTTIME)
#define FDRS_TIME_PRINTTIME TIME_PRINTTIME
#else
#define FDRS_TIME_PRINTTIME GLOBAL_TIME_PRINTTIME
#endif // TIME_PRINTTIME

// select Local Standard time Offset from UTC configuration
#if defined(STD_OFFSET)
#define FDRS_STD_OFFSET STD_OFFSET
#else
#define FDRS_STD_OFFSET GLOBAL_STD_OFFSET
#endif // STD_OFFSET

// select Local savings time Offset from UTC configuration
#if defined(DST_OFFSET)
#define FDRS_DST_OFFSET DST_OFFSET
#else
#define FDRS_DST_OFFSET GLOBAL_DST_OFFSET
#endif // DST_OFFSET

// US DST Start - 2nd Sunday in March - 02:00 local time
// US DST End - 1st Sunday in November - 02:00 local time

// EU DST Start - last Sunday in March - 01:00 UTC
// EU DST End - last Sunday in October - 01:00 UTC

time_t now;                           // Current time - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
struct timeval tv;
char strftime_buf[64];
bool validTimeFlag = false;           // Indicate whether we have reliable time
time_t lastTimeSetEvent = 0; 
bool isDST;
time_t previousTime = 0;
long slewSecs = 0;
double stdOffset = (FDRS_STD_OFFSET * 60 * 60);  // UTC -> Local time, in Seconds, offset from UTC in Standard Time
double dstOffset = (FDRS_DST_OFFSET * 60 * 60); // -1 hour for DST offset from standard time (in seconds)
time_t lastUpdate = 0;
time_t lastTimeSend = 0;
time_t lastDstCheck = 0;

// Function prototypes
void loadFDRS(float, uint8_t, uint16_t);
bool sendFDRS();

// Checks to make sure the time is valid
// Returns true if time is valid and false if not valid
bool validTime() {
  if(now < 1677000000 || (millis() - lastTimeSetEvent > (24*60*60*1000))) {
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


// If the time is valid, print the time
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
  // DBG("UTC date/time: " + String(strftime_buf));
  // now += localOffset;

  // Local time
  time_t local = time(NULL) + (isDST?dstOffset:stdOffset);
  localtime_r(&local, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  DBG("Local date/time is: " + String(strftime_buf) + (isDST?" DST":" STD"));
}

// Checks for DST or STD and adjusts time if there is a change
void checkDST() {
  if(validTime() && (time(NULL) - lastDstCheck > 5)) {
    lastDstCheck = time(NULL);
    int dstFlag = -1;
    localtime_r(&now, &timeinfo);
    if(timeinfo.tm_mon == 2) {
      struct tm dstBegin;
      dstBegin.tm_year = timeinfo.tm_year;
      dstBegin.tm_mon = 2;
#ifdef USDST
      dstBegin.tm_mday = 8;
      dstBegin.tm_hour = 2;
      dstBegin.tm_min = 0;
      dstBegin.tm_sec = 0;
      mktime(&dstBegin); // calculate tm_wday
      dstBegin.tm_mday = dstBegin.tm_mday + ((7 - dstBegin.tm_wday) % 7);
      // mktime(&dstBegin); // recalculate tm_wday
      // strftime(buf, sizeof(buf), "%c", &dstBegin);
      // DBG("DST Begins: " + String(buf) + " local");
      time_t tdstBegin = mktime(&dstBegin) - stdOffset;
#endif // USDST
#ifdef EUDST
      dstBegin.tm_mday = 25;
      dstBegin.tm_hour = 1;
      dstBegin.tm_min = 0;
      dstBegin.tm_sec = 0;
      mktime(&dstBegin); // calculate tm_wday
      dstBegin.tm_mday = dstBegin.tm_mday + ((7 - dstBegin.tm_wday) % 7);
      // mktime(&dstBegin); // recalculate tm_wday
      // strftime(buf, sizeof(buf), "%c", &dstBegin);
      // DBG("DST Begins: " + String(buf) + " local");
      time_t tdstBegin = mktime(&dstBegin);
#endif // EUDST
      if(tdstBegin != -1 && (time(NULL) - tdstBegin >= 0) && isDST == false) { // STD -> DST
        dstFlag = 1;
      }
      else if(tdstBegin != -1 && (time(NULL) - tdstBegin < 0) && isDST == true) { // DST -> STD
        dstFlag = 0;
      }
    }
    else if(timeinfo.tm_mon == 9) {
#ifdef EUDST
      struct tm dstEnd;
      dstEnd.tm_year = timeinfo.tm_year;
      dstEnd.tm_mon = 9;
      dstEnd.tm_mday = 25;
      dstEnd.tm_hour = 1;
      dstEnd.tm_min = 0;
      dstEnd.tm_sec = 0;
      mktime(&dstEnd); // calculate tm_dow
      dstEnd.tm_mday = dstEnd.tm_mday + ((7 - dstEnd.tm_wday) % 7);
      // mktime(&dstEnd); // recalculate tm_dow
      // strftime(buf, sizeof(buf), "%c", &dstEnd);
      // DBG("DST Ends: " + String(buf)  + " local");
      time_t tdstEnd = mktime(&dstEnd);
      if(tdstEnd != -1 && (time(NULL) - tdstEnd >= 0) && isDST == true) { // DST -> STD
        dstFlag = 0;
      }
      else if(tdstEnd != -1 && (time(NULL) - tdstEnd < 0) && isDST == false) { // STD -> DST
        dstFlag = 1;
      }
#endif //EUDST
#ifdef USDST
      if(isDST == false) {
        dstFlag = 1;
      }
#endif // USDST
    }
    else if(timeinfo.tm_mon == 10) {
#ifdef USDST
      struct tm dstEnd;
      dstEnd.tm_year = timeinfo.tm_year;
      dstEnd.tm_mon = 10;
      dstEnd.tm_mday = 1;
      dstEnd.tm_hour = 2;
      dstEnd.tm_min = 0;
      dstEnd.tm_sec = 0;
      mktime(&dstEnd); // calculate tm_dow
      dstEnd.tm_mday = dstEnd.tm_mday + ((7 - dstEnd.tm_wday) % 7);
      // mktime(&dstEnd); // recalculate tm_dow
      // strftime(buf, sizeof(buf), "%c", &dstEnd);
      // DBG("DST Ends: " + String(buf)  + " local");
      time_t tdstEnd = mktime(&dstEnd) - dstOffset;
      if(tdstEnd != -1 && (time(NULL) - tdstEnd >= 0) && isDST == true) { // DST -> STD
        dstFlag = 0;
      }
      else if(tdstEnd != -1 && (time(NULL) - tdstEnd < 0) && isDST == false) { // STD -> DST
        dstFlag = 1;
      }
#endif //USDST
#ifdef EUDST
      if(isDST == true) {
        dstFlag = 0;
      }
#endif // EUDST
    }
    else if((timeinfo.tm_mon == 11 || timeinfo.tm_mon == 0 || timeinfo.tm_mon == 1) && isDST == true) {
      dstFlag = 0;
    }
    else if(timeinfo.tm_mon >= 3 && timeinfo.tm_mon <= 8 && isDST == false) {
      dstFlag = 1;
    }
    if(dstFlag == 1) {
      isDST = true;
      DBG("Time change from STD -> DST");
    }
    else if(dstFlag == 0) {
      isDST = false;
      // Since we are potentially moving back an hour we need to prevent flip flopping back and forth
      // 2AM -> 1AM, wait 70 minutes -> 2:10AM then start DST checks again.
      lastDstCheck += ((65-timeinfo.tm_min) * 60); // skip checks until after beginning of next hour
      DBG("Time change from DST -> STD");
    }
  }
  return;
}

// Sets the time and calculates time time difference, in seconds, of the time change
// Returns true if time is valid otherwise false
bool setTime(time_t currentTime) {
  slewSecs = 0;
  time_t previousTime = now;

  if(currentTime != 0) {
    now = currentTime;
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
  
  // loadFDRS(now, STATUS_T, READING_ID);
  // loadFDRS(slewSecs, STATUS_T, READING_ID);
  // DO NOT CALL sendFDRS here.  Will not work for some reason ?????????
  if(validTime()) {
    lastTimeSetEvent = millis();
    return true;
  }
  else {
    return false;
  }
}

// Periodically updates the time "now"  and time struct from the internal processor time clock
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
}

void adjTimeforNetDelay(time_t newOffset) {
  static time_t previousOffset = 0;
  updateTime();
  // check to see if offset and current time are valid
  if(newOffset < UINT32_MAX && validTimeFlag) {
    now = now + newOffset - previousOffset;
    previousOffset = newOffset;
    DBG("Time adj by " + String(newOffset) + " secs");
  }
}