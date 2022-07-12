//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//

#include "fdrs_gateway_config.h"
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
#include <WiFiUdp.h>
#endif
#ifdef USE_LORA
#include <LoRa.h>
#endif
#ifdef USE_LED
#include <FastLED.h>
#endif
#ifdef USE_SD_LOG
#include <SPI.h>
#include <SD.h>
#endif
#ifdef USE_FS_LOG
#include <LittleFS.h>
#endif
#if defined (USE_SD_LOG) || defined (USE_FS_LOG)
#include <time.h>
#endif
//#include <fdrs_functions.h>  //Use global functions file
#include "fdrs_functions.h"  //Use local functions file

void setup() {
#if defined(ESP8266)
  Serial.begin(115200);
#elif defined(ESP32)
  Serial.begin(115200);
  UART_IF.begin(115200, SERIAL_8N1, RXD2, TXD2);
#endif
  DBG("Address:" + String (UNIT_MAC, HEX));
#ifdef USE_LED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB::Blue;
  FastLED.show();
#endif
#ifdef USE_WIFI
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    DBG("Connecting to WiFi...");
    DBG(FDRS_WIFI_SSID);

    delay(500);
  }
  DBG("WiFi Connected");
  client.setServer(mqtt_server, mqtt_port);
  if (!client.connected()) {
    reconnect(5);
  }
  client.setCallback(mqtt_callback);
#else
  begin_espnow();
#endif
#ifdef USE_LORA
  begin_lora();
#endif
#ifdef USE_SD_LOG
  begin_SD();
#endif
#ifdef USE_FS_LOG
  begin_FS();
#endif
  
  //DBG(sizeof(DataReading));
#ifdef USE_WIFI
   client.publish(TOPIC_STATUS, "FDRS initialized");
#endif
}

void loop() {
  #ifdef ESPNOWG_DELAY
  if ((millis() - timeESPNOWG) >= ESPNOWG_DELAY) {
    timeESPNOWG = millis();
    if  (lenESPNOWG > 0) releaseESPNOW(0);
  }
  #endif
  #ifdef ESPNOW1_DELAY
  if ((millis() - timeESPNOW1) >= ESPNOW1_DELAY) {
    timeESPNOW1 = millis();
    if (lenESPNOW1 > 0)   releaseESPNOW(1);
  }
  #endif
  #ifdef ESPNOW2_DELAY
  if ((millis() - timeESPNOW2) >= ESPNOW2_DELAY) {
    timeESPNOW2 = millis();
    if (lenESPNOW2 > 0) releaseESPNOW(2);
  }
  #endif
  #ifdef SERIAL_DELAY
  if ((millis() - timeSERIAL) >= SERIAL_DELAY) {
    timeSERIAL  = millis();
    if (lenSERIAL  > 0) releaseSerial();
  }
  #endif
  #ifdef MQTT_DELAY
  if ((millis() - timeMQTT) >= MQTT_DELAY) {
    timeMQTT = millis();
    if (lenMQTT    > 0) releaseMQTT();
  }
  #endif
  #ifdef LORAG_DELAY
  if ((millis() - timeLORAG) >= LORAG_DELAY) {
    timeLORAG = millis();
    if (lenLORAG    > 0) releaseLoRa(0);
  }
  #endif
  #ifdef LORA1_DELAY
  if ((millis() - timeLORA1) >= LORA1_DELAY) {
    timeLORA1 = millis();
    if (lenLORA1    > 0) releaseLoRa(1);
  }
  #endif
  #ifdef LORA2_DELAY
  if ((millis() - timeLORA2) >= LORA2_DELAY) {
    timeLORA2 = millis();
    if (lenLORA2    > 0) releaseLoRa(2);
  }
  #endif
  #if defined (USE_SD_LOG) || defined (USE_FS_LOG)
  if ((millis() - timeLOGBUF) >= LOGBUF_DELAY){
    timeLOGBUF = millis();
    if (logBufferPos > 0) releaseLogBuffer();
  }
  #endif

  while (UART_IF.available()) {
    getSerial();
  }
  getLoRa();
  #ifdef USE_WIFI
  if (!client.connected()) {
    reconnect(1, true);
  }
  client.loop(); // for recieving incoming messages and maintaining connection

  #endif
  if (newData) {
    switch (newData) {
      case event_espnowg:
        ESPNOWG_ACT
        break;
      case event_espnow1:
        ESPNOW1_ACT
        break;
      case event_espnow2:
        ESPNOW2_ACT
        break;
      case event_serial:
        SERIAL_ACT
        break;
      case event_mqtt:
        MQTT_ACT
        break;
      case event_lorag:
        LORAG_ACT
        break;
      case event_lora1:
        LORA1_ACT
        break;
      case event_lora2:
        LORA2_ACT
        break;
    }
    newData = 0;
  }
}
