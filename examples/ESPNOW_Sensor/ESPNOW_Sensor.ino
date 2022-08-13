//  FARM DATA RELAY SYSTEM
//
//  ESP-NOW Sensor Example
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  An example of how to send data via ESP-NOW using FDRS.
//

#include "fdrs_sensor_config.h"
#include <fdrs_node.h>

float data1;
float data2;

void setup() {
  beginFDRS();
}
void loop() {
  data1 = readHum();
  loadFDRS(data1, HUMIDITY_T);
  data2 = readTemp();
  loadFDRS(data2, TEMP_T);
  sendFDRS();
  sleepFDRS(10);  //Sleep time in seconds
}

float readTemp() {
  return 22.069;
}

float readHum() {
  return random(0,100);
}
