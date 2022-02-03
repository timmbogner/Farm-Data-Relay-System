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


uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};
uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DataReading incData[31];
DataReading theData[31];
DataReading theCommands[31];
bool newCMD = false;
int wait_time = 0;
int tot_readings = 0;
int tot_commands = 0;


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
  uint8_t incMAC[6];
  memcpy(&incData, incomingData, len);
  memcpy(&mac, incMAC, 6);
  //if (memcmp(&incMAC, &nextAddress, 6) == 0)
  int pkt_readings = len / sizeof(DataReading);
  Serial.println(":Packet:");
  for (byte i = 0; i < pkt_readings; i++) {
    Serial.println("SENSOR ID: " + String(incData[i].id) + "  TYPE " + String(incData[i].t) + "  DATA " + String(incData[i].d));

    if (incData[i].t < 200) {

      if (tot_readings >= 250 / sizeof(DataReading)) {
        Serial.println("ERROR::Too many sensor readings sent within delay period.");
        break;
      }
      theData[tot_readings] = incData[i];  //Save the current incoming reading to the next open packet slot
      ++tot_readings;
    } else {
      theCommands[tot_commands] = incData[i];  //Save the current incoming reading to the next open packet slot
      ++tot_commands;
      newCMD = true;
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
  esp_now_add_peer(broadcast_mac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

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
  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
  Serial.println();
  Serial.println("FARM DATA RELAY SYSTEM :: Terminal Module");
  Serial.println("MAC:" + WiFi.macAddress());
  Serial.print("Next device: ");
  Serial.println(NEXT_MAC);
  //  Serial.println(250 / sizeof(DataReading), DEC);
  //  Serial.println(sizeof(DataReading), DEC);
}

void loop() {
  if (newCMD) sendCmd();
  if (millis() > wait_time) {
    wait_time = wait_time + DELAY;
    if (tot_readings != 0) passForward();
  }
}

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

void sendCmd() {
  newCMD = false;
  esp_now_send(broadcast_mac, (uint8_t *) &theCommands, tot_commands* sizeof(DataReading));
  tot_commands = 0;

}
