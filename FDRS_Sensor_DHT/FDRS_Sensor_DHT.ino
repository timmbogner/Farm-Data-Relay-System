//  FARM DATA RELAY SYSTEM
//  
//  DHT11/DHT22 SENSOR MODULE
//  
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Setup instructions available in the "topography.h" file.

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DHTesp.h"
#define SENSOR_ID 2

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00};
int next_send = 0;

DHTesp dht;

typedef struct DataReading {
  float t;
  float h;
  byte n;
} DataReading;

DataReading theData;


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void loadData() {
  theData.h = dht.getHumidity();
  theData.t = dht.getTemperature() * 1.8 + 32;
  theData.n = SENSOR_ID;
}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  dht.setup(2, DHTesp::DHT22);
}

void loop() {
  if (millis() > next_send) {
    loadData();
    esp_now_send(broadcastAddress, (uint8_t *) &theData, sizeof(theData));
    next_send = millis() + 15000;
  }
}
