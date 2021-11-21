//  FARM DATA RELAY SYSTEM
//
//  BLYNK FRONT-END MODULE
//  Uses JSON data from the serial port to set Blynk variables.
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  This code was written for the now-outdated version of Blynk, and is mostly here for reference.
//  

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
#include "DataReading.h"

BlynkTimer timer;
WidgetTerminal terminal(V69);

DataReading allData[256];
int wait_time = DELAY;
char auth[] = BLKAUTH;
const char* ssid     = STASSID;
const char* password = STAPSK;
bool is_new[255];
int new_readings;
bool term_flag;
String the_command;

BLYNK_WRITE(V69)
{
  Serial.println("blynkwrite");
  the_command = param.asStr();
  term_flag = true;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println("FDRS Blynk Module");
  Serial.println("Connect to WiFi please?");
  Blynk.begin(auth, ssid, password);
  //  Blynk.config(auth);
  //  Blynk.connectWiFi(ssid, password);
  Serial.println("Thanks!");
  //terminal.clear();
  timer.setInterval(30000L, updateBlynk);
  Blynk.run();
  terminal.println("Hello, world!");
  terminal.flush();
}

void loop() {

  while (Serial.available()) {
    Serial.println("waiting for tilda");
    if (Serial.read() == '~') {     //Waits for '~', then parses following data
      getSerial();
      Serial.println("getSerial done");
      break;
    }
  }
  //  if (millis() > wait_time) {
  //    wait_time = wait_time + DELAY;
  //    if (Blynk.connected()) {
  //      updateBlynk();
  //      Serial.println("blynk updated");
  //    }
  //  }
  if (term_flag) {
    parseBlynkTerm(the_command);
    term_flag = false;
  }
  Blynk.run();
  timer.run();
}

void getSerial() {
  String incomingString = Serial.readString();
  StaticJsonDocument<3072> doc;
  Serial.println("Received: " + incomingString);
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  } else {
    for (int i = 0; i < doc.size(); i++) {
      Serial.println(doc.size());
      DataReading newData;
      newData.id = doc[i]["id"];
      newData.t = doc[i]["type"];
      newData.d = doc[i]["data"];
      allData[newData.id] = newData;
      is_new[newData.id] = true;
      Serial.println("SENSOR ID: " + String(newData.id) + "  TYPE " + String(newData.t) + "  DATA " + String(newData.d));
      yield();
    }
  }
}

void updateBlynk() {
  Serial.println("Updateblynk");
  for (int i = 0; i < 127; i++) {
    if (is_new[i] == true) {
      Serial.println("abt to write");
      Blynk.virtualWrite(i,  allData[i].d);
      is_new[i] = false;
      yield();
    }
  }
}
void parseBlynkTerm(String command) {
  int ind1 = command.indexOf(" ");
  String arg1 = command.substring(0, ind1);
  if (arg1.equalsIgnoreCase(String("SET"))) {
    int ind2 = command.indexOf(" ", ind1 + 1);
    String arg2 = command.substring(ind1 + 1, ind2);
    String arg3 = command.substring(ind2 + 1, command.length());
    terminal.println("ARG1:" + arg1);
    terminal.println("ARG2:" + arg2);
    terminal.println("ARG3:" + arg3);
    terminal.flush();
    DataReading newCMD;
    newCMD.id = arg2.toInt();
    newCMD.t = 201;
    newCMD.d = arg3.toFloat();
    allData[newCMD.id] = newCMD;
    StaticJsonDocument<2048> doc;
    doc[0]["id"]   = newCMD.id;
    doc[0]["type"] = newCMD.t;
    doc[0]["data"] = newCMD.d;
    Serial.write('~');
    serializeJson(doc, Serial);
    Serial.println();
  } else if (arg1.equalsIgnoreCase(String("GET"))) {
    String arg2 = command.substring(ind1 + 1, command.length());
    terminal.println("DataReading " + arg2 + " :: " + String(allData[arg2.toInt()].d));
    terminal.flush();
  }
}
