//  FARM DATA RELAY SYSTEM
//
//  CAPACITIVE SOIL MOISTURE SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  Connect sensor to the analog pin of the ESP (A0).
//

#include "fdrs_node_config.h"
#include <fdrs_node.h>
void setup() {
  Serial.begin(115200);
  beginFDRS();
  delay(50); //let the sensor warm up
}
void loop() {
  uint16_t s = analogRead(0);
  loadFDRS(s, SOIL_T);
  sendFDRS();
  sleepFDRS(1800);
}
