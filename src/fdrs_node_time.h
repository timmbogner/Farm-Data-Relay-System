// select Local Offset from UTC configuration
#if defined(LOCAL_OFFSET)
#define FDRS_LOCAL_OFFSET LOCAL_OFFSET
#else
#define FDRS_LOCAL_OFFSET GLOBAL_LOCAL_OFFSET
#endif // LOCAL_OFFSET

time_t now;                           // Current time - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
struct timeval tv;
char strftime_buf[64];
time_t localOffset = (FDRS_LOCAL_OFFSET * 60 * 60);  // UTC -> Local time in Seconds in Standard Time
bool validTimeFlag = false;           // Indicate whether we have reliable time
time_t lastTimeSetEvent = 0; 

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

void updateTime() {
  static time_t lastUpdate = 0;
  if(millis() - lastUpdate > 500) {
      time(&now);
      localtime_r(&now, &timeinfo);
      tv.tv_sec = now;
      tv.tv_usec = 0;
      validTime();
      lastUpdate = millis();
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
  // DBG("UTC date/time: " + String(strftime_buf));
  // now += localOffset;

  // Local time
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  DBG("Local date/time: " + String(strftime_buf));
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