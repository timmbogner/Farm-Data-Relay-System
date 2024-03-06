#include <sys/time.h>

#define MIN_TS 1709000000 // Time in Unit timestamp format should be greater than this number to be valid
#define MAX_TS 3318000000 // time in Unit timestamp format should be less than this number to be valid
#define VALID_TS(_unixts) ( (_unixts > MIN_TS && _unixts < MAX_TS) ? true : false )

#define TDIFF(prevMs,durationMs) (millis() - prevMs > durationMs)
#define TDIFFRAND(prevMs,durationMs) (millis() - prevMs > (durationMs + random(0,10000)))
#define TDIFFSEC(prevMs,durationSec) (millis() - prevMs > (durationSec * 1000))
#define TDIFFMIN(prevMs,durationMin) (millis() - prevMs > (durationMin * 60 * 1000))

// select Time, in minutes, between sending out time
#if defined(TIME_SEND_INTERVAL)
#define FDRS_TIME_SEND_INTERVAL TIME_SEND_INTERVAL
#else
#define FDRS_TIME_SEND_INTERVAL GLOBAL_TIME_SEND_INTERVAL
#endif // TIME_SEND_INTERVAL

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

time_t now;                           // Current time in UTC - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
struct timeval tv;
bool validTimeFlag = false;           // Indicate whether we have reliable time 
bool validRtcFlag = false;            // Is RTC date and time valid?
bool isDST;                           // Keeps track of Daylight Savings Time vs Standard Time
long slewSecs = 0;                  // When time is set this is the number of seconds the time changes
double stdOffset = (FDRS_STD_OFFSET * 60 * 60);  // UTC -> Local time, in Seconds, offset from UTC in Standard Time
double dstOffset = (FDRS_DST_OFFSET * 60 * 60); // -1 hour for DST offset from standard time (in seconds)
time_t lastDstCheck = 0;
unsigned long lastTimeSend = 0;
unsigned long lastRtcCheck = 0;
unsigned long lastRtcTimeSetMin = 0;

// function prototypes
void sendTimeLoRa();
void printTime();
esp_err_t sendTimeESPNow();
bool setTime(time_t);

#ifdef USE_RTC_DS3231
#include <RtcDS3231.h>
RtcDS3231<TwoWire> rtc(Wire);
#elif defined(USE_RTC_DS1307)
#include <RtcDS3231.h>
RtcDS3231<TwoWire> rtc(Wire);
#endif

#ifdef USE_RTC
void begin_rtc() {
  DBG("Starting RTC");
  rtc.Begin();

  // Is Date and time valid?
  if(!rtc.IsDateTimeValid()) {
    uint8_t err = rtc.LastError();
    if(err != 0) {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        DBG1("RTC error: Date and Time not valid! Err: " + String(err));
        validRtcFlag = false;
    }
  }
  else {
    validRtcFlag = true;
  }
  // Is RTC running?
  if(!rtc.GetIsRunning()) {
    uint8_t err = rtc.LastError();
    if(err != 0) {
      DBG1("RTC was not actively running, starting now. Err: " + String(err));
      rtc.SetIsRunning(true);
      validRtcFlag = false;
    }
  }

  if(validRtcFlag && timeSource.tmSource <= TMS_RTC && validRtcFlag) {
    if(timeSource.tmSource < TMS_RTC) {
      timeSource.tmSource = TMS_RTC;
      timeSource.tmNetIf = TMIF_LOCAL;
      timeSource.tmAddress = 0xFFFF;
      DBG1("Time source is now local RTC");
    }

    // Set date and time on the system
    DBG1("Using Date and Time from RTC.");
    if(setTime(rtc.GetDateTime().Unix32Time())) {
      timeSource.tmLastTimeSet = millis();
      printTime();
    }
  }
  
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

  lastRtcCheck = millis();
}
#endif // USE_RTC

bool validTime() {
  if(!VALID_TS(now)) {
    if(validTimeFlag) {
      DBG1("Time no longer reliable.");
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
  if(validTime()) {
    char strftime_buf[64];

    // UTC Time
    // // print Unix time:
    // //DBG2("Unix time = " + String(now));
    // localtime_r(&now, &timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    // DBG2("The current UTC date/time is: " + String(strftime_buf));

    // Local time
    time_t local = time(NULL) + (isDST?dstOffset:stdOffset);
    localtime_r(&local, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    DBG("Local date/time is: " + String(strftime_buf) + (isDST?" DST":" STD"));
  }
}

void checkDST() {
  if(validTime() && (TDIFF(lastDstCheck,5000) || lastDstCheck == 0)) {    
    lastDstCheck = millis();
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
      // DBG2("DST Begins: " + String(buf) + " local");
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
      // DBG2("DST Begins: " + String(buf) + " local");
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
      // DBG2("DST Ends: " + String(buf)  + " local");
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
      // DBG2("DST Ends: " + String(buf)  + " local");
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
      DBG1("Time change from STD -> DST");
    }
    else if(dstFlag == 0) {
      isDST = false;
      // Since we are potentially moving back an hour we need to prevent flip flopping back and forth
      // 2AM -> 1AM, wait 70 minutes -> 2:10AM then start DST checks again.
      lastDstCheck += ((65-timeinfo.tm_min) * 60); // skip checks until after beginning of next hour
      DBG1("Time change from DST -> STD");
    }
  }
  return;
}

// Periodically send time to ESP-NOW or LoRa nodes associated with this gateway/controller
void sendTime() {
  if(validTime()) { // Only send time if it is valid
    DBG1("Sending out time");
#if defined(USE_WIFI) || defined(USE_ETHERNET)    
    sendTimeSerial();
#endif    
    sendTimeLoRa();
    sendTimeESPNow();
  }
}

// time parameter is in Unix Time format UTC time zone
bool setTime(time_t currentTime) {
  slewSecs = 0;
  time_t previousTime = now;
  
  if(!VALID_TS(currentTime)) {
    return false;
  }
  now = currentTime;
  slewSecs = now - previousTime;
  if(slewSecs > 2) {
    DBG1("Time adjust " + String(slewSecs) + " secs");
  }

  // time(&now);
  localtime_r(&now, &timeinfo); // write to timeinfo struct
  mktime(&timeinfo); // set tm_isdst flag
  // Check for DST/STD time and adjust accordingly
  checkDST();
  tv.tv_sec = now;
#if defined(ESP32) || defined(ESP8266) // settimeofday may only work with Espressif chips
  settimeofday(&tv,NULL); // set the RTC time
#endif
#ifdef USE_RTC
// Only set the RTC time every 60 minutes in order to prevent flash wear
  if(TDIFFMIN(lastRtcTimeSetMin,60)) {
    RtcDateTime rtcNow;
    rtcNow.InitWithUnix32Time(now);
    rtc.SetDateTime(rtcNow);
  }
#endif
  // Uncomment below to send time and slew rate to the MQTT server
  // loadFDRS(now, TIME_T, 111);
  // loadFDRS(slewSecs, TIME_T, 111);
  // Do not call sendFDRS here.  It will not work for some reason.
  if(validTime()) {
    if(FDRS_TIME_SEND_INTERVAL == 0 && (TDIFF(lastTimeSend,5000) || lastTimeSend == 0)) { // avoid sending twice on start with RTC and WiFi
      sendTime();
      lastTimeSend = millis();
    }
    return true;
  }
  return false;
}

void handleTime() {
  static unsigned long lastUpdate = 0;

  if(TDIFF(lastUpdate,500)) {
    time(&now);
    localtime_r(&now, &timeinfo);
    tv.tv_sec = now;
    tv.tv_usec = 0;
    validTime();
    checkDST();
    lastUpdate = millis();
  }

#ifdef USE_RTC  
  // If RTC was not running or had the incorrect date on startup recheck periodically.
  if(!validRtcFlag && (TDIFFMIN(lastRtcCheck,60))) {
    begin_rtc();
    lastRtcCheck = millis();
  }
#endif // USE_RTC

  // Send out time to other devices if we have exceeded the time send interval
  if(validTimeFlag && (FDRS_TIME_SEND_INTERVAL != 0) && TDIFFMIN(lastTimeSend,FDRS_TIME_SEND_INTERVAL)) {
    lastTimeSend = millis();
    sendTime();
  }
  if(timeSource.tmNetIf < TMIF_LOCAL && TDIFFMIN(timeSource.tmLastTimeSet,120)) { // Reset time source to default if not heard anything for two hours
    timeSource.tmNetIf = TMIF_NONE;
    timeSource.tmAddress = 0x0000;
    timeSource.tmLastTimeSet = millis();
    timeSource.tmSource = TMS_NONE;
  }
}

void adjTimeforNetDelay(time_t newOffset) {
  static time_t previousOffset = 0;

  // check to see if offset and current time are valid
  if(newOffset < UINT32_MAX && validTime()) {
    now = now + newOffset - previousOffset;
    previousOffset = newOffset;
    if(newOffset > 2) {
      DBG1("Time adj by " + String(newOffset) + " secs");
    }
  }
  if(timeSource.tmSource == TMS_NET && newOffset > 10) {
    DBG("Time off by more than 10 seconds!");
    // loadFDRS();
  }
}