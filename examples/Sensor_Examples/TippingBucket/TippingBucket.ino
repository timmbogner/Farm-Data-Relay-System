//  FARM DATA RELAY SYSTEM
//
//  TIPPING BUCKET RAINFALL SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.

#define REED_PIN 2

#include "fdrs_node_config.h"
#include <fdrs_node.h>

unsigned int theCount = 0;
unsigned long lastTrigger = 0;
boolean clicked = false;

ICACHE_RAM_ATTR void detectsMovement() {
  clicked = true;
  lastTrigger = millis();
}

void setup() {
beginFDRS();
  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), detectsMovement, FALLING);
}

void loop() {
  if (clicked && millis() - lastTrigger > 100) {
    theCount++;
    Serial.print("DINK.");
    Serial.println(theCount);
    clicked = false;
    loadFDRS(theCount, RAINFALL_T);
    sendFDRS();
  }

}
