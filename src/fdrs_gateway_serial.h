#include <ArduinoJson.h>

#if defined (ESP32)
  #define UART_IF Serial1
  #ifdef USE_GPS
    #define GPS_IF Serial2
  #endif
#elif defined (ESP8266)
  #define UART_IF Serial
#else
  #define UART_IF Serial
  #ifdef USE_GPS
    #define GPS_IF Serial1
  #endif
#endif

#if defined(ESP32)
#if !defined RXD2 or !defined TXD2
    #warning Defining RXD2 and TXD2 using MCU defaults.
    #if CONFIG_IDF_TARGET_ESP32
        #define RXD2 14
        #define TXD2 15
    #elif CONFIG_IDF_TARGET_ESP32S2 or CONFIG_IDF_TARGET_ESP32S3
        #define RXD2 18
        #define TXD2 17
    #elif CONFIG_IDF_TARGET_ESP32C3
        #define RXD2 2
        #define TXD2 3
    #else
        #error MCU not supported.
    #endif
#endif
#endif

extern time_t now;

bool gpsParse(String input) {
  String _time, _date;
  int pos;
  struct tm gpsDateTime;
  static unsigned long lastGpsTimeSet = 0;

  if(timeSource.tmSource != TMS_GPS) {
    timeSource.tmSource = TMS_GPS;
    timeSource.tmAddress = 0xFFFF;
    timeSource.tmNetIf = TMIF_LOCAL;
    DBG1("Time source is now local GPS.");
  }

  // Prints out all incoming GPS data
  // DBG2("GPS incoming: " + input);

  // $GNZDA $GNZDA,154230.000,11,02,2024,00,00*4F
  // $GNRMC $GNRMC,154230.000,A,,,,,,,110224,,,A,V*19

  if(input.startsWith("$GNZDA") && input.length() >= 38) {
    // DBG2("GPS String: " + input + " Length: " + String(input.length()));
    pos = input.indexOf(",");
    _time = input.substring(pos + 1,pos + 7);
    // DBG2("GPS Time: " + _time + " ($GNZDA)");
    pos = input.indexOf(",",pos + 5);
    _date = input.substring(pos,pos + 11);
    _date.replace(",","");
    // DBG2("GPS Date: " + _date + " ($GNZDA)");

    // Set GPS time every 10 minutes
    if(lastGpsTimeSet == 0 || TDIFFMIN(lastGpsTimeSet,10)) {    
      lastGpsTimeSet = millis();
      pos = 0;
      gpsDateTime.tm_hour = _time.substring(pos,pos+2).toInt();
      pos+=2;
      gpsDateTime.tm_min = _time.substring(pos,pos+2).toInt();
      pos+=2;
      gpsDateTime.tm_sec = _time.substring(pos,pos+2).toInt();
      pos = 0;
      gpsDateTime.tm_mday = _date.substring(pos,pos+2).toInt();
      pos+=2;
      gpsDateTime.tm_mon = _date.substring(pos,pos+2).toInt() - 1;
      pos+=2;
      gpsDateTime.tm_year = _date.substring(pos,pos+4).toInt() - 1900;
      DBG2("GPS Date & Time: " + String(gpsDateTime.tm_mon + 1) + "/" + String(gpsDateTime.tm_mday) + "/" + String(gpsDateTime.tm_year + 1900) + " " \
        + String(gpsDateTime.tm_hour) + ":" + String(gpsDateTime.tm_min) + ":" + String(gpsDateTime.tm_sec) + " UTC");
      DBG1("Setting date and time via GPS: " + String(mktime(&gpsDateTime)) + " $GNZDA");
      if(setTime(mktime(&gpsDateTime))) {
        timeSource.tmLastTimeSet = millis();
        return true;
      }
    }
  }
  else if(input.startsWith("$GNRMC") && input.length() >= 38) {

    // DBG2("GPS String: " + input + " Length: " + String(input.length()));
    pos = input.indexOf(",");
    _time = input.substring(pos + 1,pos + 7);
    // DBG2("GPS Time: " + _time + " ($GNRMC)");
    for(int i = 0 ; i < 8; i++) {
      pos = input.indexOf(",",pos + 1); 
    }
    _date = input.substring(pos,pos + 9);
    _date.replace(",","");
    // DBG2("GPS Date: " + _date + " ($GNRMC)");
    if(lastGpsTimeSet == 0 || TDIFFMIN(lastGpsTimeSet,10)) {    
      lastGpsTimeSet = millis();
      pos = 0;
      gpsDateTime.tm_hour = _time.substring(pos,pos+2).toInt();
      pos+=2;
      gpsDateTime.tm_min = _time.substring(pos,pos+2).toInt();
      pos+=2;
      gpsDateTime.tm_sec = _time.substring(pos,pos+2).toInt();
      pos = 0;
      gpsDateTime.tm_mday = _date.substring(pos,pos+2).toInt();
      pos+=2;
      gpsDateTime.tm_mon = _date.substring(pos,pos+2).toInt() - 1;
      pos+=2;
      gpsDateTime.tm_year = 2000 + _date.substring(pos,pos+2).toInt() - 1900;
      DBG2("GPS Date & Time: " + String(gpsDateTime.tm_mon + 1) + "/" + String(gpsDateTime.tm_mday) + "/" + String(gpsDateTime.tm_year + 1900) + " " \
        + String(gpsDateTime.tm_hour) + ":" + String(gpsDateTime.tm_min) + ":" + String(gpsDateTime.tm_sec) + " UTC");
      DBG1("Setting date and time via GPS: " + String(mktime(&gpsDateTime)) + " $GNRMC");
      if(setTime(mktime(&gpsDateTime))) {
        timeSource.tmLastTimeSet = millis();
        return true;
      }
    }   
  }
  return false;
}

void getSerial() {
  String incomingString;

  if (UART_IF.available()){
   incomingString =  UART_IF.readStringUntil('\n');
  }
  else if (Serial.available()){
   incomingString =  Serial.readStringUntil('\n');
  }
#ifdef GPS_IF
  if (GPS_IF.available()){
   
   // Data is coming in every second from the GPS, let's minimize the processing power
   // required by only parsing periodically - maybe every 60 seconds.
   static unsigned long lastGpsParse = 0;
   if(lastGpsParse == 0 || TDIFFSEC(lastGpsParse,60)) {
      lastGpsParse = millis();
      for(int i=0; i < 20; i++) {
      incomingString =  GPS_IF.readStringUntil('\n');
      if(gpsParse(incomingString)) {
        return;
      }
    }
   }
   return;
  }
#endif // GPS_IF
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    DBG2("json parse err");
    DBG2(incomingString);
    return;
  } else {
    int s = doc.size();
    JsonObject obj = doc[0].as<JsonObject>();
    if(obj.containsKey("type")) { // DataReading
    //UART_IF.println(s);
    for (int i = 0; i < s; i++) {
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = event_serial;
    DBG("Incoming Serial");
      String data;
      serializeJson(doc, data);
      DBG1("DR data: " + data);
    }
    else if(obj.containsKey("cmd")) { // SystemPacket
      cmd_t c = doc[0]["cmd"];
      uint32_t p = doc[0]["param"];
      if(c == cmd_time && p > MIN_TS) {
        if(timeSource.tmNetIf < TMIF_SERIAL) {
          timeSource.tmNetIf = TMIF_SERIAL;
          timeSource.tmSource = TMS_NET;
          timeSource.tmAddress = 0xFFFF;
          DBG1("Time source is now Serial peer");
        }
        if(timeSource.tmNetIf == TMIF_SERIAL) { 
        DBG1("Incoming Serial: time");
if(setTime(doc[0]["param"])) {
        timeSource.tmLastTimeSet = millis();
      }
      else {
        // Set time failed for some reason
          }
        }
        else {
          // There is a local time source so we do not accept serial
          DBG2("Did not set time from incoming serial.");
        }
      }
      else if(c == cmd_time && p == 0) {
        // Received a request for us to send time via serial -- not implemented yet.
      }
      else {
        DBG2("Incoming Serial: unknown cmd: " + String(c));
      }
    }
    else {    // Who Knows???
      DBG2("Incoming Serial: unknown: " + incomingString);
    }
  }
}


void sendSerial() {
  
  JsonDocument doc;
  for (int i = 0; i < ln; i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
  }
  DBG("Sending Serial.");
  // String data;
  // serializeJson(doc, data);
  // DBG("Serial data: " + data);

  serializeJson(doc, UART_IF);
  UART_IF.println();

#ifndef ESP8266
  serializeJson(doc, Serial);
  Serial.println();
#endif

}
void handleSerial(){
#ifdef GPS_IF
  while (UART_IF.available() || Serial.available() || GPS_IF.available())
#else
  while (UART_IF.available() || Serial.available())
#endif
  {
    getSerial();
  }
}

void sendTimeSerial() {
  
  JsonDocument SysPacket;
  SysPacket[0]["cmd"]   = cmd_time;
  SysPacket[0]["param"] = now;
  serializeJson(SysPacket, UART_IF);
  UART_IF.println();
  DBG("Sending Time via Serial.");
// String serialData;
  // DBG2("Serial data: " + serializeJson(SysPacket, serialData));

#ifndef ESP8266
  // serializeJson(SysPacket, Serial);
  // Serial.println();
#endif
}

void begin_gps() {
#ifdef GPS_IF
#ifdef ARDUINO_ARCH_SAMD
    GPS_IF.begin(9600);
  #else
    GPS_IF.begin(9600, SERIAL_8N1, GPS_RXD, GPS_TXD);
  #endif  
#endif
}