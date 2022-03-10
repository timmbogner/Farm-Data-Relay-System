//  FARM DATA RELAY SYSTEM
//
//  "fdrs_sensor.h"
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
#define READING_ID    1   //Unique ID for sensor module
#define GTWY_MAC      0x00 //Address of the nearest gateway

#define POWER_CTRL    14
#define MAC_PREFIX    0xAA, 0xBB, 0xCC, 0xDD, 0xEE
//#define DEEP_SLEEP
#define USE_ESPNOW

//#define USE_LORA
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

#define STATUS_T    0  // Status 
#define TEMP_T      1  // Temperature 
#define TEMP2_T     2  // Temperature #2
#define HUMIDITY_T  3  // Relative Humidity 
#define PRESSURE_T  4  // Atmospheric Pressure 
#define LIGHT_T     5  // Light (lux) 
#define SOIL_T      6  // Soil Moisture 
#define SOIL2_T     7  // Soil Moisture #2 
#define SOILR_T      8 // Soil Resistance 
#define SOILR2_T     9 // Soil Resistance #2 
#define OXYGEN_T    10 // Oxygen 
#define CO2_T       11 // Carbon Dioxide
#define WINDSPD_T   12 // Wind Speed
#define WINDHDG_T   13 // Wind Direction
#define RAINFALL_T  14 // Rainfall
#define MOTION_T    15 // Motion
#define VOLTAGE_T   16 // Voltage
#define VOLTAGE2_T  17 // Voltage #2
#define CURRENT_T   18 // Current
#define CURRENT2_T  19 // Current #2
#define IT_T        20 // Iterations


#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#include <LoRa.h>


const uint16_t espnow_size = 250 / sizeof(DataReading);
uint8_t gatewayAddress[] = {MAC_PREFIX, GTWY_MAC};
uint8_t gtwyAddress[] = {gatewayAddress[3], gatewayAddress[4], GTWY_MAC};
uint8_t LoRaAddress[] = {0x42, 0x00};


uint32_t wait_time = 0;
DataReading fdrsData[espnow_size];
uint8_t data_count = 0;

void beginFDRS() {
#ifdef USE_ESPNOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
#ifdef POWER_CTRL
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
#endif
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#if defined(ESP8266)
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  // Register peers
  esp_now_add_peer(gatewayAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Register first peer
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
#endif
#ifdef USE_LORA
#ifndef __AVR__
  SPI.begin(SCK, MISO, MOSI, SS);
#endif
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    while (1);
  }
#endif
}
void transmitLoRa(uint8_t* mac, DataReading * packet, uint8_t len) {
#ifdef USE_LORA
  uint8_t pkt[5 + (len * sizeof(DataReading))];
  memcpy(&pkt, mac, 3);
  memcpy(&pkt[3], &LoRaAddress, 2);
  memcpy(&pkt[5], packet, len * sizeof(DataReading));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
#endif
}
void sendFDRS() {
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *) &fdrsData, data_count * sizeof(DataReading));
  delay(5);
#endif
#ifdef USE_LORA
  transmitLoRa(gtwyAddress, fdrsData, data_count);
#endif
  data_count = 0;
}
void loadFDRS(float d, uint8_t t) {
  if (data_count > espnow_size) sendFDRS();
  DataReading dr;
  dr.id = READING_ID;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;

}
void sleepFDRS(int sleep_time) {
#ifdef DEEP_SLEEP
#ifdef ESP32
  esp_sleep_enable_timer_wakeup(sleep_time * 1000000);
  esp_deep_sleep_start();
#endif
#ifdef ESP8266
  ESP.deepSleep(sleep_time * 1000000);
#endif
#endif
  delay(sleep_time * 1000);

}
