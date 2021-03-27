//  FARM DATA RELAY SYSTEM
//
//  BLYNK FRONT-END MODULE
//  Uses JSON data from the serial port to set Blynk variables.
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.

#define STASSID ""   //Your SSID
#define STAPSK  ""   //Your Password
#define BLKAUTH ""   //Your Blynk auth code
#define DELAY 30000

#if defined(ESP8266)
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#endif
#include <ArduinoJson.h>

typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

DataReading allData[256];
int wait_time = DELAY;
char auth[] = BLKAUTH;
const char* ssid     = STASSID;
const char* password = STAPSK;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println("FDRS Blynk Module");
  Serial.println("Connect to WiFi please?");
  Blynk.begin(auth, ssid, password);
  Serial.println("Thanks!");
}

void loop() {
  Blynk.run();
  if (Serial.available()) getSerial();
  if (millis() > wait_time) {
    wait_time = wait_time + DELAY;
    updateBlynk();
  }
}

void getSerial() {
  String incomingString = Serial.readString();
  StaticJsonDocument<3072> doc;
  //Serial.println(incomingString);
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  } else {
    for (int i = 0; i < 30; i++) {
      DataReading newData;
      newData.id = doc[i]["id"];
      newData.t = doc[i]["type"];
      newData.d = doc[i]["data"];
      if (newData.t != 0) {    //Only save if type is not zero
        allData[newData.id] = newData; 
        Serial.println("SENSOR ID: " + String(newData.id) + "  TYPE " + String(newData.t) + "  DATA " + String(newData.d));
      }
    }
    Serial.println("Parsed");
  }
}

void updateBlynk() {
  Serial.println("Updating Blynk!");
  Blynk.virtualWrite(V0,  allData[0].d);
  Blynk.virtualWrite(V1,  allData[1].d);
  Blynk.virtualWrite(V2,  allData[2].d);
  Blynk.virtualWrite(V3,  allData[3].d);
  Blynk.virtualWrite(V4,  allData[4].d);
  Blynk.virtualWrite(V5,  allData[5].d);
  Blynk.virtualWrite(V6,  allData[6].d);
  Blynk.virtualWrite(V7,  allData[7].d);
  Blynk.virtualWrite(V8,  allData[8].d);
  Blynk.virtualWrite(V9,  allData[9].d);
  Blynk.virtualWrite(V10, allData[10].d);
  Blynk.virtualWrite(V11, allData[11].d);
}
