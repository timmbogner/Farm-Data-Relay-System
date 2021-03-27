//  FARM DATA RELAY SYSTEM
//
//  TERMINAL MODULE
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

typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

typedef struct DataPacket {
  uint8_t l;
  DataReading packet[30];

} DataPacket;

DataPacket incData;
DataPacket theData;
bool newData = false;
int wait_time = 0;
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};

void passForward() {
  Serial.println("Passing On");
  esp_now_send(nextAddress, (uint8_t *) &theData, sizeof(theData));
  for (int i = 0; i < 30; i++) {
    theData.packet[i].d = 0;
    theData.packet[i].id = 0;
    theData.packet[i].t = 0;
    theData.l = 0;
  }
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
  memcpy(&incData, incomingData, sizeof(incData));
  Serial.println(":Packet:");
  for (byte i = 0; i < incData.l; i++) {
    Serial.println("SENSOR ID: " + String(incData.packet[i].id) + "  TYPE " + String(incData.packet[i].t) + "  DATA " + String(incData.packet[i].d));
    if (theData.l >= 30) {
      Serial.println("ERROR::Too many sensor readings sent within delay period.");
      break;
    }
    theData.packet[theData.l] = incData.packet[i];  //Save the current incoming reading to the next open packet slot
    ++theData.l;
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  // Init WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
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
  esp_now_add_peer(nextAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
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
  // Register peer
  memcpy(peerInfo.peer_addr, nextAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
  Serial.println();
  Serial.println("Sola Gratia FDRS Terminal");
  Serial.println("MAC:" + WiFi.macAddress());
  Serial.print("Next device: ");
  Serial.println(NEXT_MAC);
}

void loop() {
  if (millis() > wait_time) {
    wait_time = wait_time + DELAY;
    if (theData.l != 0) passForward();
  }
}
