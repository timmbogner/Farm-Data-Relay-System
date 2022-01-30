//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#include "fdrs_config.h"
#include <ArduinoJson.h>
#include "DataReading.h"
#include <PubSubClient.h>
#include "fdrs_functions.h"
#include <LoRa.h>

#ifdef USE_WIFI
const char* ssid = WIFI_NET;
const char* password = WIFI_PASS;
const char* mqtt_server = MQTT_ADDR;
#endif


void setup() {
#if defined(ESP8266)
  Serial.begin(115200);
#elif defined(ESP32)
  Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);
#endif
  begin_espnow();
#ifdef USE_WIFI
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
#endif
#ifdef USE_LORA
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    while (1);
  }
#endif
}

void loop() {
  if (millis() > timeESPNOWG) {
    timeESPNOWG += ESPNOWG_DELAY;
    if  (lenESPNOWG > 0) releaseESPNOW(0);
  }
  if (millis() > timeESPNOW1) {
    timeESPNOW1 += ESPNOW1_DELAY;
    if (lenESPNOW1 > 0)   releaseESPNOW(1);
  }
  if (millis() > timeESPNOW2) {
    timeESPNOW2 += ESPNOW2_DELAY;
    if (lenESPNOW2 > 0) releaseESPNOW(2);
  }
  if (millis() > timeSERIAL) {
    //Serial.println("timeSERIAL tripped: " + String(lenSERIAL));
    timeSERIAL  += SERIAL_DELAY;
    if (lenSERIAL  > 0) releaseSerial();
  }
  if (millis() > timeMQTT) {
    timeMQTT += MQTT_DELAY;
    if (lenMQTT    > 0) releaseMQTT();
  }
  if (millis() > timeLORA) {
    timeLORA += LORA_DELAY;
    if (lenLORA    > 0) releaseLoRa();
  }

  while (Serial.available()) {
    getSerial();
  }
#ifdef USE_LORA
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    LoRa.readBytes((uint8_t *)&theData, packetSize);
    ln = packetSize;
    newData = 6;
  }
#endif
#ifdef USE_WIFI
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
#endif
  if (newData) {
    switch (newData) {
      case 1:     //ESP-NOW #1
        ESPNOW1_ACT
        break;
      case 2:     //ESP-NOW #2
        ESPNOW2_ACT
        break;
      case 3:     //ESP-NOW General
        ESPNOWG_ACT
        break;
      case 4:     //Serial
        SERIAL_ACT
        break;
      case 5:     //MQTT
        MQTT_ACT
        break;
      case 6:     //LoRa
        LORA_ACT
        break;
    }
    newData = 0;
  }
}
