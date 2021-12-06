//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000
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
#include <ArduinoJson.h>
#include "DataReading.h"
#include <PubSubClient.h>
#include "fdrs_functions.h"

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.0.8";

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
        sendESPNOW(2);
        sendSerial();
        break;
      case 2:     //ESP-NOW #2
        sendESPNOW(1);
        sendSerial();
        break;
      case 3:     //ESP-NOW General
        sendSerial();
        break;
      case 4:     //Serial
        sendESPNOW(0);
        sendSerial();
        sendMQTT();
        break;
      case 5:     //MQTT
        sendSerial();
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
