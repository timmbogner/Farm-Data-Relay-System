//  FARM DATA RELAY SYSTEM
//  
//  DHT11/DHT22 SENSOR MODULE
//  
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a one-byte identifier.

#define SENSOR_ID 0
#define TERM_MAC 0x00 //Terminal MAC


#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DHTesp.h"

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};
int next_send = 0;

DHTesp dht;

typedef struct DataReading {
  float t;
  float h;
  byte n;
  byte d;

} DataReading;

DataReading theData;

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
  float h = dht.getHumidity();
  float t = dht.getTemperature();
  if (!(isnan(h) || isnan(t))) {
  theData.h =   h;
  theData.t =  t * 1.8 + 32;
  theData.n = SENSOR_ID;
  theData.d = 1;
  }else{
  theData.h =   -4.20;
  theData.t =  -69.00;
  theData.n = SENSOR_ID;
  theData.d = -1;
  }
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
