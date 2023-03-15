// Example code from https://docs.arduino.cc/tutorials/ethernet-shield-rev2/udp-ntp-client

#include <WiFiUdp.h>

// select NTP Time Server configuration
#if defined(TIME_SERVER)
#define FDRS_TIME_SERVER TIME_SERVER
#else 
#define FDRS_TIME_SERVER GLOBAL_TIME_SERVER
#endif // TIME_SERVER

// select Local Offset from UTC configuration
#if defined(LOCAL_OFFSET)
#define FDRS_LOCAL_OFFSET LOCAL_OFFSET
#else
#define FDRS_LOCAL_OFFSET GLOBAL_LOCAL_OFFSET
#endif // LOCAL_OFFSET

// select Time, in minutes, between NTP time server queries configuration
#if defined(TIME_FETCHNTP)
#define FDRS_TIME_FETCHNTP TIME_FETCHNTP
#else
#define FDRS_TIME_FETCHNTP GLOBAL_TIME_FETCHNTP
#endif // TIME_FETCHNTP

// select Time, in minutes, between time printed configuration
#if defined(TIME_PRINTTIME)
#define FDRS_TIME_PRINTTIME TIME_PRINTTIME
#else
#define FDRS_TIME_PRINTTIME GLOBAL_TIME_PRINTTIME
#endif // TIME_PRINTTIME
#define DSTSTART  (timeinfo.tm_mon == 10 && timeinfo.tm_wday == 0 && timeinfo.tm_mday < 8 && timeinfo.tm_hour == 2)
#define DSTEND    (timeinfo.tm_mon == 2 && timeinfo.tm_wday == 0 && timeinfo.tm_mday > 7 && timeinfo.tm_mday < 15 && timeinfo.tm_hour == 2)

WiFiUDP FDRSNtp;
unsigned int localPort = 8888;        // local port to listen for UDP packets
const char timeServer[] = FDRS_TIME_SERVER; // NTP server
const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];   //buffer to hold incoming and outgoing packets
time_t now;                           // Current time - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
struct timeval tv;
char strftime_buf[64];
time_t localOffset = (FDRS_LOCAL_OFFSET * 60 * 60);  // UTC -> Local time in Seconds in Standard Time
bool validTimeFlag = false;           // Indicate whether we have reliable time 
uint NTPFetchFail = 0;                // consecutive NTP fetch failures
time_t lastNTPFetchSuccess = 0;      // Last time that a successful NTP fetch was made
bool isDST;                           // Keeps track of Daylight Savings Time vs Standard Time
long slewSecs = 0;                  // When time is set this is the number of seconds the time changes

// Function prototypes
void loadFDRS(float, uint8_t, uint16_t);
void sendFDRS();

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  FDRSNtp.beginPacket(address, 123); // NTP requests are to port 123
  FDRSNtp.write(packetBuffer, NTP_PACKET_SIZE);
  FDRSNtp.endPacket();
}

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

bool setTime(time_t previousTime) {
  
  // Adjust for local time
  now += localOffset;
  slewSecs = now - previousTime;
  DBG("Time adjust " + String(slewSecs) + " secs");

  // time(&now);
  tv.tv_sec = now;
  settimeofday(&tv,NULL);
  localtime_r(&now, &timeinfo);
  // Check for DST/STD time and adjust accordingly
  checkDST();
  // Uncomment below to send time and slew rate to the MQTT server
  // loadFDRS(now, STATUS_T, 111);
  // loadFDRS(slewSecs, STATUS_T, 111);
  // sendFDRS();
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

#ifdef USE_LORA
  sendTimeLoRa();
#endif

#ifdef USE_ESPNOW
  sendTimeESPNow();
#endif

}

void updateTime() {
  static time_t lastUpdate = 0;
  static time_t lastTimeSend = 0;

  if(millis() - lastUpdate > 500) {
    time(&now);
    localtime_r(&now, &timeinfo);
    tv.tv_sec = now;
    tv.tv_usec = 0;
    validTime();
    checkDST();
    lastUpdate = millis();
  }
  if(validTimeFlag && (millis() - lastTimeSend) > TIME_SEND_INTERVAL_MS) {
    sendTime();
    lastTimeSend = millis();
  }
}

void fetchNtpTime() {
  time_t previousTime = 0;

  if(WiFi.status() == WL_CONNECTED) {
    FDRSNtp.begin(localPort);

    sendNTPpacket(timeServer); // send an NTP packet to a time server
    uint i = 0;
    for(i = 0; i < 800; i++) {
      if(FDRSNtp.parsePacket())
        break;
      delay(10);
    }
    if(i < 800) {
      DBG("Took " + String(i * 10) + "ms to get NTP response from " + String(timeServer) + ".");
      NTPFetchFail = 0;
      // We've received a packet, read the data from it
      FDRSNtp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      // the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, extract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      //DBG("Seconds since Jan 1 1900 = " + String(secsSince1900));
      
      // now convert NTP time into everyday time:
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      // now is epoch format - seconds since Jan 1 1970
      previousTime = now;
      now = secsSince1900 - seventyYears;
      setTime(previousTime);
    }
    else {
      NTPFetchFail++;
      DBG("Timeout getting a NTP response. " + String(NTPFetchFail) + " consecutive failures.");
      // If unable to Update the time after N tries then set the time to be not valid.
      if(NTPFetchFail > 5) {
        validTimeFlag = false;
        DBG("Time no longer reliable.");
      }
    }
  }
}

void begin_ntp() {
  fetchNtpTime();
  updateTime();
  printTime();
}