//  FARM DATA RELAY SYSTEM
//
//  CAPACITIVE SOIL MOISTURE SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  Connect the sensor to an analog pin of your MCU.
//

#define SOIL_PIN  36  // Ignored on ESP8266

#include "fdrs_node_config.h"
#include <fdrs_node.h>
void setup() {
  beginFDRS();
  delay(50); //let the sensor warm up
}
void loop() {
#ifdef ESP8266
  uint16_t s = analogRead(0);
#else
  uint16_t s = analogRead(SOIL_PIN);
#endif
  loadFDRS(s, SOIL_T);
  sendFDRS();
  sleepFDRS(60 * 5);
}
