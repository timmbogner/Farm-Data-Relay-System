//  FARM DATA RELAY SYSTEM
//
//  GATEWAY MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Setup instructions located in the "fdrs_config.h" file.

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "fdrs_config.h"
#include <ArduinoJson.h>

uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
typedef struct DataReading {
  float t;
  float h;
  byte n;
  byte d;

} DataReading;


DataReading theData[6];

bool newData = false;

void encodeJSON() {
  StaticJsonDocument<512> doc;
  for (int i = 0; i < 6; i++) {
    doc[i]["id0"] = theData[i].n;
    doc[i]["id1"] = theData[i].d;
    doc[i]["data0"] = theData[i].t;
    doc[i]["data1"] = theData[i].h;
  }
  serializeJson(doc, Serial);
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (sendStatus == 0) {
  }
  else {
  }
}

void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&theData, incomingData, sizeof(theData));
  newData = true;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  // Init WiFi and set MAC address
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_macaddr(STATION_IF, selfAddress);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
}

void loop() {
  if (newData) {
    newData = false;
    encodeJSON();
  }
}
