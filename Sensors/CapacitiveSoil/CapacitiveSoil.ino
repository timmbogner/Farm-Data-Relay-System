//  FARM DATA RELAY SYSTEM
//
//  CAPACITIVE SOIL MOISTURE SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type
//
#define CALIBRATION_MODE 0
#define CALIB_H 285
#define CALIB_L 485
#define TERM_MAC    0x00 //Terminal MAC
#define SLEEPYTIME  60   //Time to sleep in seconds
#define SOIL_ID     51    //Unique ID (0 - 255) for each data reading

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DataReading.h"


uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
  ESP.deepSleep(30e6);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

  loadData();
}
void loop() {

}
void loadData() {
  uint16_t s = analogRead(0);
  if (!CALIBRATION_MODE) {
    if (s < CALIB_H) s = CALIB_H;
    if (s > CALIB_L) s = CALIB_L;
    s = map(s, CALIB_H, CALIB_L, 100, 0);
  }
  Serial.println();
  Serial.println(s);
  DataReading Soil;
  Soil.d = s;
  Soil.id = SOIL_ID;
  Soil.t = 5;
  DataReading thePacket[1];
  thePacket[0] = Soil;
  esp_now_send(broadcastAddress, (uint8_t *) &thePacket, sizeof(thePacket));
}
