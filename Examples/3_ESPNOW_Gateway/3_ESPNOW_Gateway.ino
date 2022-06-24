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

#ifdef USE_LED
CRGB leds[NUM_LEDS];
#endif


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


#ifdef LORA_PEER_1
uint8_t LoRa1[6] =         {MAC_PREFIX, LORA_PEER_1};
#endif

#ifdef LORA_PEER_2
uint8_t LoRa2[6] =         {MAC_PREFIX, LORA_PEER_2};
#endif


MQTT_FDRSGateWay MQTT(WIFI_SSID,WIFI_PASS,MQTT_ADDR,MQTT_PORT);

ESP_FDRSGateWay ESPNow;

#if defined(ESP32)
Serial_FDRSGateWay SerialGW(&Serial1,115200);
#else
Serial_FDRSGateWay SerialGW(&Serial,115200);
#endif

LoRa_FDRSGateWay LoRaGW(MISO,MOSI,SCK,SS,RST,DIO0,BAND,SF);

void setup() {

#ifdef USE_LED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Blue;
  FastLED.show();
#endif

  MQTT.init();

  ESPNow.init(selfAddress);

#ifdef ESPNOW_PEER_1
  ESPNow.add_peer(ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  ESPNow.add_peer(ESPNOW2);
#endif

#if defined(ESP32)
  SerialGW.init(SERIAL_8N1,RXD2,TXD2);
#else
  SerialGW.init();
#endif

#ifdef USE_LORA
  LoRaGW.init(selfAddress);
#endif

#ifdef ESPNOW_PEER_1
  ESPNow.add_peer(ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  ESPNow.add_peer(ESPNOW2);
#endif
  
}

void loop() {

  LoRaGW.get();
  SerialGW.get();

  ESPNow.release();
  MQTT.release();
  SerialGW.release();
  LoRaGW.release();

  //It does not matter witch one you call.
  //it will clear all the data that has been received.
  ESPNow.flush();
  MQTT.flush();
  SerialGW.flush();
  LoRaGW.flush();

}
