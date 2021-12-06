//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000
//  This is still in progress. Stay tuned!
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

const char* ssid = WIFI_NET;
const char* password = WIFI_PASS;
const char* mqtt_server = MQTT_ADDR;

void setup() {
  Serial.begin(115200);
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
}

void loop() {
  while (Serial.available()) {
    getSerial();
  }
  if (newData != 0) {

    
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
    }
    newData = 0;
  }


#ifdef USE_WIFI
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
#endif
}
