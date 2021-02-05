//  FARM DATA RELAY SYSTEM
//
//  RELAY MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Setup instructions located in the "fdrs_config.h" file.

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "fdrs_config.h"

uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};
uint8_t incMAC[6];
uint8_t outMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

uint8_t theData[250];
bool newData = false;

void passOn() {
  if (incMAC[5] == PREV_MAC) outMAC[5] = NEXT_MAC;
  if (incMAC[5] == NEXT_MAC) outMAC[5] = PREV_MAC;
  Serial.print("Packet Received from device: ");
  Serial.println(incMAC[5]);
  Serial.print("and sent to: ");
  Serial.println(outMAC[5]);

  esp_now_send(outMAC, (uint8_t *) &theData, sizeof(theData));
}

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
  memcpy(&theData, incomingData, sizeof(theData));
  memcpy(&incMAC, mac, sizeof(incMAC));
  Serial.print("Data received: ");
  Serial.println(len);
  newData = true;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  // Init WiFi and set MAC address
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_macaddr(STATION_IF, selfAddress);
  Serial.println();
  Serial.println("Sola Gratia FDRS Relay");
  Serial.println("New MAC:" + WiFi.macAddress());
  Serial.print("Previous device: ");
  Serial.println(PREV_MAC);
  Serial.print("Next device: ");
  Serial.println(NEXT_MAC);
  Serial.println(" ");
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  esp_now_add_peer(nextAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
}

void loop() {
  if (newData) {
    newData = false;
    passOn();
  }
}
