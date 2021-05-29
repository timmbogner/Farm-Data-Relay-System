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
#include "DataReading.h"

DataReading incData[31];
DataReading theData[31];
DataReading theCMD;
byte mac_table[256];


bool newCMD = false;
int wait_time = 0;
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};
int tot_readings = 0;
void passForward() {
  Serial.println("Passing Forward");
  esp_now_send(nextAddress, (uint8_t *) &theData, sizeof(DataReading)*tot_readings);
  tot_readings = 0;
  for (int i = 0; i < 250 / sizeof(DataReading); i++) {
    theData[i].d = 0;
    theData[i].id = 0;
    theData[i].t = 0;
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
  memcpy(&incData, incomingData, len);
  int pkt_readings = len / sizeof(DataReading);
  Serial.println(":Packet:");
  for (byte i = 0; i < pkt_readings; i++) {
    Serial.println("SENSOR ID: " + String(incData[i].id) + "  TYPE " + String(incData[i].t) + "  DATA " + String(incData[i].d));
    if (tot_readings >= 250 / sizeof(DataReading)) {
      Serial.println("ERROR::Too many sensor readings sent within delay period.");
      break;
    }
    switch (incData[i].t) {
      case 200:
        Serial.println("Registering " + String(mac[5]) +" to ID: " + String(incData[i].id));
        mac_table[incData[i].id] =  mac[5];
        break;
      case 201:
        newCMD = true;
        theCMD = incData[i];
        break;
        
      default:
        theData[tot_readings] = incData[i];  //Save the current incoming reading to the next open packet slot
        ++tot_readings;
        break;
    }
  }
}

void setup() {
  // Init Serial 
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
//  Serial.println(250 / sizeof(DataReading), DEC);
//  Serial.println(sizeof(DataReading), DEC);
}

void loop() {
  if (newCMD) {
    newCMD = false;
    sendCmd();
  }
  if (millis() > wait_time) {
    wait_time = wait_time + DELAY;
    if (tot_readings != 0) passForward();
  }
}

void sendCmd() {
  uint8_t sendAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, mac_table[theCMD.id]};
//#if defined(ESP8266)
//  esp_now_add_peer(nextAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
//#elif defined(ESP32)
//  esp_now_peer_info_t peerInfo;
//  peerInfo.channel = 0;
//  peerInfo.encrypt = false;
//  // Register peer
//  memcpy(peerInfo.peer_addr, nextAddress, 6);
//  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//    Serial.println("Failed to add peer");
//    return;
//  }
//#endif
  Serial.println("Sending CMD " + String(theCMD.d) + " to " + String(theCMD.id) + " at " + String(sendAddress[5], HEX));
  esp_now_send(sendAddress, (uint8_t *) &theCMD, sizeof(DataReading));
}
