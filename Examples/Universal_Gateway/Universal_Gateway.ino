//  FARM DATA RELAY SYSTEM
//
//  Experimental Universal Gateway
//
//  Under construction, feedback is appreciated!
//

#include "fdrs_config.h"

#ifdef USE_LED
#include <FastLED.h>
#endif
#include "fdrs_gateway.h"
#include "fdrs_config.h"

#ifdef USE_LED
CRGB leds[NUM_LEDS];
#endif


uint8_t selfAddress[6] =   {MAC_PREFIX, UNIT_MAC};

#if defined(ESP_GET) || defined(ESP_SEND)

#ifdef ESPNOW_PEER_1
uint8_t ESPNOW1[] =       {MAC_PREFIX, ESPNOW_PEER_1};
#endif

#ifdef ESPNOW_PEER_2
uint8_t ESPNOW2[] =       {MAC_PREFIX, ESPNOW_PEER_2};
#endif

#endif


#if defined(LORA_GET) || defined(LORA_SEND)

#ifdef LORA_PEER_1
uint8_t LoRa1[6] =         {MAC_PREFIX, LORA_PEER_1};
#endif

#ifdef LORA_PEER_2
uint8_t LoRa2[6] =         {MAC_PREFIX, LORA_PEER_2};
#endif

#endif


#if defined(MQTT_GET) || defined(MQTT_SEND)

MQTT_FDRSGateWay MQTT(WIFI_SSID,WIFI_PASS,MQTT_ADDR,MQTT_PORT);

#endif

#if defined(ESP_GET) || defined(ESP_SEND)
ESP_FDRSGateWay ESPNow;
#endif

#if defined(SER_GET) || defined(SER_SEND)

#if defined(ESP32)
Serial_FDRSGateWay SerialGW(&Serial1,115200);
#else
Serial_FDRSGateWay SerialGW(&Serial,115200);
#endif

#endif

#if defined(LORA_GET) || defined(LORA_SEND)

LoRa_FDRSGateWay LoRaGW(MISO,MOSI,SCK,SS,RST,DIO0,BAND,SF);

#endif


void setup() {

#ifdef USE_LED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Blue;
  FastLED.show();
#endif

#if defined(MQTT_GET) || defined(MQTT_SEND)
  MQTT.init();
#endif

#if defined(ESP_GET) || defined(ESP_SEND)
  ESPNow.init(selfAddress);

#ifdef ESPNOW_PEER_1
  ESPNow.add_peer(ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  ESPNow.add_peer(ESPNOW2);
#endif

#endif


#if defined(SER_GET) || defined(SER_SEND)

#if defined(ESP32)
  SerialGW.init(SERIAL_8N1,RXD2,TXD2);
#else
  SerialGW.init();
#endif

#endif

#if defined(LORA_GET) || defined(LORA_SEND)
  LoRaGW.init(selfAddress);
#endif

}

void loop() {

#if defined(LORA_GET)
  LoRaGW.get();
#endif

#if defined(SER_GET)
  SerialGW.get();
#endif

#if defined(ESP_SEND)
#ifdef ESPNOW_ALL
  ESPNow.release();
#endif
#ifdef ESPNOW_PEER_1
  ESPNow.release(ESPNOW1);
#endif
#ifdef ESPNOW_PEER_2
  ESPNow.release(ESPNOW2);
#endif
#endif

#if defined(MQTT_SEND)
  MQTT.release();
#endif

#if defined(SER_SEND)
  SerialGW.release();
#endif

#if defined(LORA_SEND)
  LoRaGW.release();
#endif

//It does not matter witch one you call.
//it will clear all the data that has been received.
#if defined(ESP_SEND)
  ESPNow.flush();
#endif

#if defined(MQTT_SEND)
  MQTT.flush();
#endif

#if defined(SER_SEND)
  SerialGW.flush();
#endif

#if defined(LORA_SEND)
  LoRaGW.flush();
#endif

}
