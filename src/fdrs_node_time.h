// select Local Offset from UTC configuration
#if defined(LOCAL_OFFSET)
#define FDRS_LOCAL_OFFSET LOCAL_OFFSET
#else
#define FDRS_LOCAL_OFFSET GLOBAL_LOCAL_OFFSET
#endif // LOCAL_OFFSET
#define DSTSTART  (timeinfo.tm_mon == 10 && timeinfo.tm_wday == 0 && timeinfo.tm_mday < 8 && timeinfo.tm_hour == 2)
#define DSTEND    (timeinfo.tm_mon == 2 && timeinfo.tm_wday == 0 && timeinfo.tm_mday > 7 && timeinfo.tm_mday < 15 && timeinfo.tm_hour == 2)

time_t now;                           // Current time - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
struct timeval tv;
char strftime_buf[64];
bool validTimeFlag = false;           // Indicate whether we have reliable time
time_t lastTimeSetEvent = 0; 
bool isDST;
time_t previousTime = 0;
long slewSecs = 0;

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
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  DBG("Local date/time: " + String(strftime_buf));
}

// Checks for DST or STD and adjusts time if there is a change
void checkDST() {
  // DST -> STD - add one hour (3600 seconds)
  if(validTimeFlag && isDST && (DSTEND || timeinfo.tm_isdst == 0)) {
    isDST = false;
    now += 3600;
    DBG("Time change from DST -> STD");
  }
  // STD -> DST - subtract one hour (3600 seconds)
  else if(validTimeFlag && !isDST && (DSTSTART || timeinfo.tm_isdst == 1)) {
    isDST = true;
    now -= 3600;
    DBG("Time change from STD -> DST");
  }
  return;
}

// Sets the time and calculates time time difference, in seconds, of the time change
// Returns true if time is valid otherwise false
bool setTime(time_t previousTime) {
  
  slewSecs = now - previousTime;
  DBG("Time adjust " + String(slewSecs) + " secs");

  // time(&now);
  tv.tv_sec = now;
  settimeofday(&tv,NULL);
  localtime_r(&now, &timeinfo);
  // Check for DST/STD time and adjust accordingly
  checkDST();
  // Uncomment below to send time and slew rate to the MQTT server
  // loadFDRS(now, STATUS_T, READING_ID);
  // loadFDRS(slewSecs, STATUS_T, READING_ID);
  // using sendFDRS below in LoRa node seems to have issues with not hearing ACKs from GW.  Use sendFDRS in main loop.
  // sendFDRS();
  if(validTime()) {
    lastTimeSetEvent = millis();
    printTime();
    return true;
  }
  else {
    return false;
  }
}

// Periodically updates the time "now"  and time struct from the internal processor time clock
void updateTime() {
  static time_t lastUpdate = 0;
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