//  FARM DATA RELAY SYSTEM
//  
//  TERMINAL MODULE
//  
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Setup instructions located in the "fdrs_config.h" file.

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "fdrs_config.h"

#define DELAY 60000

uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};

typedef struct DataReading {
  float t;
  float h;
  byte n;
  byte d;
} DataReading;

DataReading incData;
DataReading theData[6];

bool newData = false;
int wait_time = 0;

void passForward() {
  Serial.println("Passing On");
  esp_now_send(nextAddress, (uint8_t *) &theData, sizeof(theData));
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");}
  else {
    Serial.println("Delivery fail");
  }

}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incData, incomingData, sizeof(incData));
  theData[incData.n] = incData;
  Serial.println(":Packet:");
  Serial.print("Temp:");
  Serial.println(incData.t);
  Serial.print("Humidity:");
  Serial.println(incData.h);
  Serial.print("ID:");
  Serial.println(incData.n);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  //Init WiFi and set MAC address
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println();
  Serial.println("Sola Gratia FDRS Terminal");
  Serial.print("Original MAC: ");
  Serial.println(WiFi.macAddress());
  wifi_set_macaddr(STATION_IF, selfAddress);
  Serial.println("New MAC:" + WiFi.macAddress());
  Serial.print("Next device: ");
  Serial.println(NEXT_MAC);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_add_peer(nextAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
}

void loop() {
  if (millis() > wait_time) {
    wait_time = wait_time + DELAY;
    passForward();
  }
}
