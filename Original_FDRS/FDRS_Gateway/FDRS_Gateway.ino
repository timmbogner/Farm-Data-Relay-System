//  FARM DATA RELAY SYSTEM
//
//  GATEWAY MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Setup instructions located in the "fdrs_config.h" file.

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#define RXD2 21
#define TXD2 22
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#include "fdrs_config.h"
#include <ArduinoJson.h>
#include "DataReading.h"

uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t prevAddress[] = {MAC_PREFIX, PREV_MAC};
uint8_t selfAddress[] = {MAC_PREFIX, UNIT_MAC};
DataReading incData[31];
bool newData = false;
int pkt_readings;
uint8_t incMAC[6];

#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif

  memcpy(&incData, incomingData, len);
  memcpy(&incMAC, mac, 6);
  pkt_readings = len / sizeof(DataReading);
  newData = true;
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#if defined(ESP8266)
  Serial.begin(115200);
  wifi_set_macaddr(STATION_IF, selfAddress);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  esp_now_add_peer(broadcast_mac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

#elif defined(ESP32)
  Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  esp_wifi_set_mac(WIFI_IF_STA, &selfAddress[0]);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, prevAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
}

//CMD example {"id":72,"type":201,"data":166}

void getSerial() {
  DataReading theCommands[31];
  String incomingString =  Serial.readStringUntil('\n');
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    Serial.println("parse err");

    return;
  } else {
    int s = doc.size();
    //Serial.println(s);
    for (int i = 0; i < s; i++) {
      if (i > 31) break;
      theCommands[i].id = doc[i]["id"];
      theCommands[i].t = doc[i]["type"];
      theCommands[i].d = doc[i]["data"];
    }
    esp_now_send(broadcast_mac, (uint8_t *) &theCommands, s * sizeof(DataReading));

  }
}

void encodeJSON() {
  StaticJsonDocument<2048> doc;
  for (int i = 0; i < pkt_readings; i++) {
    doc[i]["id"]   = incData[i].id;
    doc[i]["type"] = incData[i].t;
    doc[i]["data"] = incData[i].d;
    incData[i].id = 0;
    incData[i].t = 0;
    incData[i].d = 0;
  }
  serializeJson(doc, Serial);
  Serial.println();
}

void loop() {
  while (Serial.available()) {
    getSerial();
  }
  if (newData) {
    newData = false;
    encodeJSON();
  }
}
