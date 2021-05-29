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
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#include "fdrs_config.h"
#include <ArduinoJson.h>
#include "DataReading.h"

uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
DataReading incData[31];
DataReading theData[31];
bool newData = false;
int pkt_readings;
void encodeJSON() {
  StaticJsonDocument<2048> doc;
  for (int i = 0; i < pkt_readings; i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
    theData[i].id = 0;
    theData[i].t = 0;
    theData[i].d = 0;
  }
  Serial.write('~');
  serializeJson(doc, Serial);
}

#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  memcpy(&theData, incomingData, len);
  pkt_readings = len / sizeof(DataReading);
  newData = true;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  // Init WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println();
  Serial.println("FDRS Gateway Module");
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#if defined(ESP8266)
  wifi_set_macaddr(STATION_IF, selfAddress);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)
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
  // register peer
  memcpy(peerInfo.peer_addr, prevAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
}


void getSerial() {
  String incomingString = Serial.readString();
  StaticJsonDocument<3072> doc;
  //Serial.println(incomingString);
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    return;
  } else {
    DataReading newCMD;
    newCMD.id = doc[0]["id"];
    newCMD.t = doc[0]["type"];
    newCMD.d = doc[0]["data"];
    esp_now_send(prevAddress, (uint8_t *) &newCMD, sizeof(newCMD));
  }
}

void loop() {
  while (Serial.available()) {
    if (Serial.read() == '~') {
      getSerial();
    }
  }
  if (newData) {
    newData = false;
    encodeJSON();
  }
}
