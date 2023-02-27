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
time_t lastUpdate = 0;                // keep track of update time loop refresh

void updateTime() {
  if(millis() - lastUpdate > 500) {
      time(&now);
      localtime_r(&now, &timeinfo);
      tv.tv_sec = now;
      tv.tv_usec = 0;
      lastUpdate = millis();
    }
}

void printTime() {
  if(now < 1677000000) {
    //DBG("Error in NTP Response.");
    validTimeFlag = false;
    DBG("Time no longer reliable.");
    return;
  }
  validTimeFlag = true;

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