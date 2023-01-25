#include <ArduinoJson.h>

#if defined (ESP32)
#define UART_IF Serial1
#else
#define UART_IF Serial
#endif


DataReading SERIALbuffer[256];
uint8_t lenSERIAL = 0;
uint32_t timeSERIAL = 0;

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
    //UART_IF.println(s);
    for (int i = 0; i < s; i++) {
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = event_serial;
    DBG("Incoming Serial.");

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

void bufferSerial() {
  DBG("Buffering Serial.");
  for (int i = 0; i < ln; i++) {
    SERIALbuffer[lenSERIAL + i] = theData[i];
  }
  lenSERIAL += ln;
  //UART_IF.println("SENDSERIAL:" + String(lenSERIAL) + " ");
}

void releaseSerial() {
  DBG("Releasing Serial.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < lenSERIAL; i++) {
    doc[i]["id"]   = SERIALbuffer[i].id;
    doc[i]["type"] = SERIALbuffer[i].t;
    doc[i]["data"] = SERIALbuffer[i].d;
  }
  serializeJson(doc, UART_IF);
  UART_IF.println();
  lenSERIAL = 0;
}

