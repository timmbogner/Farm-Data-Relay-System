//  FARM DATA RELAY SYSTEM
//
//  TIPPING BUCKET RAINFALL SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type
//
#define REED_PIN 2
#define TERM_MAC 0x00 //Terminal MAC
#define RAIN_ID  0    //Unique ID (0 - 255) for each data reading
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DataReading.h"
unsigned int theCount = 0;
unsigned long lastTrigger = 0;
boolean clicked = false;

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}
// Checks if motion was detected, sets LED HIGH and starts a timer
ICACHE_RAM_ATTR void detectsMovement() {
  clicked = true;
  lastTrigger = millis();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), detectsMovement, FALLING);
}

void loop() {
  if (clicked && millis() - lastTrigger > 100) {
    theCount++;
    Serial.print("CLICK.");
    Serial.println(theCount);
    clicked = false;
    DataReading Rain;
    Rain.d = theCount;
    Rain.id = RAIN_ID;
    Rain.t = 7;
    esp_now_send(broadcastAddress, (uint8_t *) &Rain, sizeof(Rain));
  }

}
