//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//

#include "fdrs_config.h"



#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#include <ArduinoJson.h>
#ifdef USE_WIFI
#include <PubSubClient.h>
#endif
#ifdef USE_LORA
#include <LoRa.h>
#endif
#ifdef USE_LED
#include <FastLED.h>
#endif
#include "fdrs_gateway.h"
#include "fdrs_config.h"

#define USE_WIFI

#define ESPNOW_PEER_1  0x0E  // ESPNOW1 Address 
#define ESPNOW_PEER_2  0x0F  // ESPNOW2 Address

uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t selfAddress[6] =   {MAC_PREFIX, UNIT_MAC};


#ifdef ESPNOW_PEER_1
uint8_t ESPNOW1[] =       {MAC_PREFIX, ESPNOW_PEER_1};
#else
uint8_t ESPNOW1[] =       {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
#ifdef ESPNOW_PEER_2
uint8_t ESPNOW2[] =       {MAC_PREFIX, ESPNOW_PEER_2};
#else
uint8_t ESPNOW2[] =       {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif


MQTT_FDRSGateWay MQTT(1000,WIFI_SSID,WIFI_PASS,MQTT_ADDR,MQTT_PORT);

ESP_FDRSGateWay ESPNow(broadcast_mac,selfAddress,1000);

void setup() {

#ifdef USE_LED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Blue;
  FastLED.show();
#endif
#ifdef USE_WIFI
  MQTT.init();
#else
  ESPNow.init();

#ifdef ESPNOW_PEER_1
  ESPNow.add_peer(ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  ESPNow.add_peer(ESPNOW2);
#endif


#endif

// #ifdef USE_LORA
//   DBG("Initializing LoRa!");
//   SPI.begin(SCK, MISO, MOSI, SS);
//   LoRa.setPins(SS, RST, DIO0);
//   if (!LoRa.begin(FDRS_BAND)) {
//     while (1);
//   }
//   LoRa.setSpreadingFactor(FDRS_SF);
//   DBG(" LoRa initialized.");
// #endif
  
  //DBG(sizeof(DataReading));

}

void loop() {

  ESPNow.release();

  #ifdef ESPNOWG_DELAY
  if (millis() > timeESPNOWG) {
    timeESPNOWG += ESPNOWG_DELAY;
    if  (lenESPNOWG > 0) releaseESPNOW(0);
  }
  #endif
  #ifdef ESPNOW1_DELAY
  if (millis() > timeESPNOW1) {
    timeESPNOW1 += ESPNOW1_DELAY;
    if (lenESPNOW1 > 0)   releaseESPNOW(1);
  }
  #endif
  #ifdef ESPNOW2_DELAY
  if (millis() > timeESPNOW2) {
    timeESPNOW2 += ESPNOW2_DELAY;
    if (lenESPNOW2 > 0) releaseESPNOW(2);
  }
  #endif
  #ifdef SERIAL_DELAY
  if (millis() > timeSERIAL) {
    timeSERIAL  += SERIAL_DELAY;
    if (lenSERIAL  > 0) releaseSerial();
  }
  #endif
  #ifdef MQTT_DELAY
  if (millis() > timeMQTT) {
    timeMQTT += MQTT_DELAY;
    if (lenMQTT    > 0) releaseMQTT();
  }
  #endif

  // #ifdef LORAG_DELAY
  // if (millis() > timeLORAG) {
  //   timeLORAG += LORAG_DELAY;
  //   if (lenLORAG    > 0) releaseLoRa(0);
  // }
  // #endif
  // #ifdef LORA1_DELAY
  // if (millis() > timeLORA1) {
  //   timeLORA1 += LORA1_DELAY;
  //   if (lenLORA1    > 0) releaseLoRa(1);
  // }
  // #endif
  // #ifdef LORA2_DELAY
  // if (millis() > timeLORA2) {
  //   timeLORA2 += LORA2_DELAY;
  //   if (lenLORA2    > 0) releaseLoRa(2);
  // }
  // #endif

  while (UART_IF.available()) {
    getSerial();
  }
  // getLoRa();

}
