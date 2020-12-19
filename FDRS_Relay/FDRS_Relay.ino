//  FARM DATA RELAY SYSTEM
//  
//  RELAY MODULE
//  
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, IL.
//  Setup instructions available in the "topography.h" file.

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "topography.h"

uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};
uint8_t incMAC[6];
uint8_t outMAC[6];

typedef struct DataReading {
  float t;
  float h;
  byte n;
} DataReading;

DataReading theData[6];
bool clickStatus = false;
bool newData = false;
uint8_t senderMAC[12];

void passOn() {
  Serial.print("Packet Received from device: ");
  if (memcmp(&incMAC, &prevAddress, 6)) memcpy(&outMAC, &nextAddress, 6);
  if (memcmp(&incMAC, &nextAddress, 6)) memcpy(&outMAC, &prevAddress, 6);
  Serial.println(outMAC[5]);
  esp_now_send(outMAC, (uint8_t *) &theData, sizeof(theData));
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
    clickStatus = true;
  }
  else {
    Serial.println("Delivery fail");
    clickStatus = false;
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
  //Set an output pin for feedback (clicks)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true);
  // Init WiFi and set MAC address
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println();
  Serial.println("Sola Gratia FDRS Relay");
  Serial.print("Original MAC: ");
  Serial.println(WiFi.macAddress());
  wifi_set_macaddr(STATION_IF, selfAddress);
  Serial.println("New MAC:" + WiFi.macAddress());
  Serial.print("Previous device: ");
  Serial.println(PREV_MAC);
  Serial.print("Next device: ");
  Serial.println(NEXT_MAC);
  Serial.println(" ");
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
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
    digitalWrite(LED_BUILTIN, false);
    delay(50);
    digitalWrite(LED_BUILTIN, true);
    passOn();
  }
  if (clickStatus) {
    delay(250);
    clickStatus = false;
    digitalWrite(LED_BUILTIN, false);
    delay(250);
    digitalWrite(LED_BUILTIN, true);
  }
}
