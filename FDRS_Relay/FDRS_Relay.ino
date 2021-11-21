//  FARM DATA RELAY SYSTEM
//
//  RELAY MODULE
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

uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};
uint8_t outMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, DEFT_MAC};
uint8_t incMAC[6];

uint8_t theData[250];
uint8_t ln;
bool newData = false;

void passOn() {
  switch (incMAC[5]) {
    case PREV_MAC:
      outMAC[5] = NEXT_MAC;
      break;
    case NEXT_MAC:
      outMAC[5] = PREV_MAC;
      break;
    default:
      outMAC[5] = DEFT_MAC;
      break;
  }

  Serial.print("Packet Received from device: ");
  Serial.println(incMAC[5]);
  Serial.print("and sending to: ");
  Serial.println(outMAC[5]);
  esp_now_send(outMAC, (uint8_t *) &theData, ln);
}

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
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
void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  memcpy(&theData, incomingData, sizeof(theData));
  memcpy(&incMAC, mac, sizeof(incMAC));
  if (memcmp(&incMAC, &selfAddress, 5) != 0) return;
  Serial.print("Data received: ");
  Serial.println(len);
  ln = len;
  newData = true;
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
  // Register peers
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
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
  // Register first peer
  memcpy(peerInfo.peer_addr, prevAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register second peer
  memcpy(peerInfo.peer_addr, nextAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif

  Serial.println();
  Serial.println("FARM DATA RELAY SYSTEM :: Relay Module");
  Serial.println("MAC:" + WiFi.macAddress());
  Serial.print("Previous device: ");
  Serial.println(PREV_MAC);
  Serial.print("Next device: ");
  Serial.println(NEXT_MAC);
  Serial.print("Default device: ");
  Serial.println(DEFT_MAC);
  Serial.println(" ");
}

void loop() {
  if (newData) {
    newData = false;
    passOn();
  }
}
