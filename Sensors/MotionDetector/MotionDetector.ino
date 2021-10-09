//  FARM DATA RELAY SYSTEM
//
//  MOTION SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type
//  This code is unfinished, but may work.
//
#define MOTION_PIN 13
#define TERM_MAC 0x00 //Terminal MAC
#define MOTION_ID  100    //Unique ID (0 - 255) for each data reading
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DataReading.h"
unsigned int theCount = 0;
unsigned long nextCheck = 0;
boolean motionDetected = false;
unsigned long isr_time = millis();

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};


ICACHE_RAM_ATTR void detectsMovement() {
  if (millis() > isr_time + 15000) {
    nextCheck = millis();
    isr_time = millis();
  }
}


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

  pinMode(MOTION_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), detectsMovement, RISING);


}

void loop() {
  if ( millis() > nextCheck) {
    checkDetector();

  }
}
void checkDetector() {

  Serial.println("Checking");

  nextCheck = millis() + 15000;
  motionDetected = digitalRead(MOTION_PIN);
  if (motionDetected == HIGH) {
    Serial.println("Motion Detected");

    DataReading Motion;
    Motion.d = theCount;
    Motion.id = MOTION_ID;
    Motion.t = 8;
    esp_now_send(broadcastAddress, (uint8_t *) &Motion, sizeof(Motion));
  } else {

  }

}
