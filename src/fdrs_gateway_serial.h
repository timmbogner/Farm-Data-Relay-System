#include <ArduinoJson.h>

#if defined (ESP32)
#define UART_IF Serial1
#else
#define UART_IF Serial
#endif

extern time_t now;

void getSerial() {
  String incomingString;

  if (UART_IF.available()){
   incomingString =  UART_IF.readStringUntil('\n');
  }
  else if (Serial.available()){
   incomingString =  Serial.readStringUntil('\n');
  }
  DynamicJsonDocument doc(24576);
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    //    DBG("json parse err");
    //    DBG(incomingString);
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
      DBG("Incoming Serial: DR");
    }
    else if(obj.containsKey("cmd")) { // SystemPacket
      cmd_t c = doc[0]["cmd"];
      if(c == cmd_time) {
        setTime(doc[0]["param"]); 
        DBG("Incoming Serial: time");
      }
      else {
        DBG("Incoming Serial: unknown cmd: " + String(c));
      }
    }
    else {    // Who Knows???
      DBG("Incoming Serial: unknown");
    }
  }
}


void sendSerial() {
  DBG("Sending Serial.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < ln; i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
  }
  serializeJson(doc, UART_IF);
  UART_IF.println();

#ifndef ESP8266
  serializeJson(doc, Serial);
  Serial.println();
#endif

}
void handleSerial(){
  while (UART_IF.available() || Serial.available())
  {
    getSerial();
  }
}

void sendTimeSerial() {
  DBG("Sending Time via Serial.");
  DynamicJsonDocument SysPacket(64);
  SysPacket[0]["cmd"]   = cmd_time;
  SysPacket[0]["param"] = now;
  serializeJson(SysPacket, UART_IF);
  UART_IF.println();

#ifndef ESP8266
  // serializeJson(SysPacket, Serial);
  // Serial.println();
#endif
}