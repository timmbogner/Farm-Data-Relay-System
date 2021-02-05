//  FARM DATA RELAY SYSTEM
//
//  BLYNK FRONT-END MODULE
//  Uses json data from the serial port to set Blynk variables.
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.

//  Change the following three lines to match your project:
#define STASSID "MySSID"
#define STAPSK  "MyPassword"
#define BLKAUTH "MyBlynkAuth"

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>


char auth[] = BLKAUTH;

typedef struct DataReading {
  float t;
  float h;
  byte n;
  byte d;
} DataReading;

DataReading theData[6];

const char* ssid     = STASSID;
const char* password = STAPSK;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Blynk.begin(auth, ssid, password);
}

void loop() {
  Blynk.run();
  if (Serial.available()) getSerial();
}

void getSerial() {
  delay(50);
  StaticJsonDocument<512> doc;
  //char serial_data[512];
  String serial_data;
  int serial_len = 0;
  while (Serial.available()) {
    //serial_data[serial_len] = Serial.read();
    char c = Serial.read();
    serial_data = serial_data + char(c); 
    serial_len++;
  }
  Serial.println(serial_data);
  DeserializationError error = deserializeJson(doc, serial_data);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  } else {
    for (int i = 0; i < 6; i++) {
      theData[i].n = doc[i]["id0"];
      theData[i].d = doc[i]["id1"];
      theData[i].t = doc[i]["data0"];
      theData[i].h = doc[i]["data1"];
    }
    updateBlynk();
  }
}

void updateBlynk() {
  Blynk.run();
  Blynk.virtualWrite(V1,  theData[0].t); 
  Blynk.virtualWrite(V3,  theData[1].t);
  Blynk.virtualWrite(V5,  theData[2].t);
  Blynk.virtualWrite(V7,  theData[3].t);
  Blynk.virtualWrite(V9,  theData[4].t);
  Blynk.virtualWrite(V11, theData[5].t);
  Blynk.virtualWrite(V2,  theData[0].h);
  Blynk.virtualWrite(V4,  theData[1].h);
  Blynk.virtualWrite(V6,  theData[2].h);
  Blynk.virtualWrite(V8,  theData[3].h);
  Blynk.virtualWrite(V10, theData[4].h);
  Blynk.virtualWrite(V12, theData[5].h);
}
