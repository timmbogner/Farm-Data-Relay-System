//  FARM DATA RELAY SYSTEM
//
//  Experimental Universal Gateway
//
//  Under construction, feedback is appreciated!
//

#include "fdrs_gateway_config.h"

#ifdef USE_LED
#include <FastLED.h>
#endif
#include "fdrs_gateway.h"
#include "fdrs_gateway_config.h"

#ifdef USE_LED
CRGB leds[NUM_LEDS];
#endif

uint8_t selfAddress[6] =   {MAC_PREFIX, UNIT_MAC};

#if defined(ESP_GET) || defined(ESP_SEND)

#ifdef ESPNOW_ALL
std::vector<DataReading_t> espnow_peer_unknown_data;
#endif

#ifdef ESPNOW_PEER_1
uint8_t ESPNOW1[] =       {MAC_PREFIX, ESPNOW_PEER_1};
std::vector<DataReading_t> espnow_peer_1_data;
#endif

#ifdef ESPNOW_PEER_2
uint8_t ESPNOW2[] =       {MAC_PREFIX, ESPNOW_PEER_2};
std::vector<DataReading_t> espnow_peer_2_data;
#endif

#endif

#if defined(LORA_GET) || defined(LORA_SEND)

#ifdef LORA_PEER_1
uint8_t LoRa1[6] =         {MAC_PREFIX, LORA_PEER_1};
std::vector<DataReading_t> lora_peer_1_data;
#endif

#ifdef LORA_PEER_2
uint8_t LoRa2[6] =         {MAC_PREFIX, LORA_PEER_2};
std::vector<DataReading_t> lora_peer_2_data;
#endif

#endif

#if defined(MQTT_GET) || defined(MQTT_SEND)
MQTT_FDRSGateWay MQTT(WIFI_SSID,WIFI_PASS,MQTT_ADDR,MQTT_PORT);
// TODO: should be:
//MQTT_FDRSGateWay MQTT(GLOBAL_SSID,GLOBAL_PASS,GLOBAL_MQTT_ADDR,GLOBAL_MQTT_PORT);
std::vector<DataReading_t> mqtt_data;
#endif

#if defined(ESP_GET) || defined(ESP_SEND)
ESP_FDRSGateWay ESPNow;
#endif

#if defined(SER_GET) || defined(SER_SEND)

#ifdef ESP32
Serial_FDRSGateWay SerialGW(&Serial1,115200);
#else
Serial_FDRSGateWay SerialGW(&Serial,115200);
#endif

std::vector<DataReading_t> serial_data;

#endif

#if defined(LORA_GET) || defined(LORA_SEND)

LoRa_FDRSGateWay LoRaGW(SPI_MISO,SPI_MOSI,SPI_SCK,LORA_SS,LORA_RST,LORA_DIO0,LORA_BAND,LORA_SF);

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

#ifdef ESP32
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

//collect data
#ifdef LORA_GET
  LoRaGW.get();
#endif

#ifdef SER_GET
  SerialGW.get();
#endif

//get the collected data
#ifdef ESP_GET
#ifdef ESPNOW_ALL
  espnow_peer_unknown_data = ESPNow.get_unkown_peer_data();
#endif

#ifdef ESPNOW_PEER_1
  espnow_peer_1_data = ESPNow.get_peer_data(ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  espnow_peer_2_data = ESPNow.get_peer_data(ESPNOW2);
#endif
#endif

#ifdef MQTT_GET
  mqtt_data = MQTT.get_data();
#endif

#ifdef SER_GET
  serial_data = SerialGW.get_data();
#endif

#ifdef LORA_GET
#ifdef LORA_PEER_1
  lora_peer_1_data = LoRaGW.get_peer_data(LoRa1);
#endif
#ifdef LORA_PEER_1
  lora_peer_2_data = LoRaGW.get_peer_data(LoRa2);
#endif
#endif

//send the collected data to where you want it
#ifdef ESP_SEND
#ifdef ESPNOW_ALL
  // send ESPNOW_UNKNOWN_PEER data to ESPNOW_UNKNOWN_PEER
  ESPNow.release(espnow_peer_unknown_data);
#endif

#ifdef ESPNOW_PEER_1
  // send ESPNOW_PEER_2 data to ESPNOW_PEER_1
  ESPNow.release(espnow_peer_2_data,ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  // send ESPNOW_PEER_1 data to ESPNOW_PEER_2
  ESPNow.release(espnow_peer_1_data,ESPNOW2);
#endif
#endif

#ifdef MQTT_SEND
// send ESPNOW_PEER_1 data to MQTT
  MQTT.release(espnow_peer_1_data);
#endif

#ifdef SER_SEND
  // send ESPNOW_PEER_2 data to SERIAL
  SerialGW.release(espnow_peer_2_data);
#endif

#ifdef LORA_SEND
  // send SERIAL data to LoRa
  LoRaGW.release(serial_data);
#endif

//clear out the buffers
#ifdef ESP_GET
#ifdef ESPNOW_ALL
  ESPNow.flush();
#endif

#ifdef ESPNOW_PEER_1
  ESPNow.flush(ESPNOW1);
#endif

#ifdef ESPNOW_PEER_2
  ESPNow.flush(ESPNOW2);
#endif
#endif

#ifdef MQTT_GET
  MQTT.flush();
#endif

#ifdef SER_GET
  SerialGW.flush();
#endif

#ifdef LORA_GET
#ifdef LORA_PEER_1
  LoRaGW.flush(LoRa1);
#endif
#ifdef LORA_PEER_1
  LoRaGW.flush(LoRa2);
#endif
#endif

  delay(1000);

}
