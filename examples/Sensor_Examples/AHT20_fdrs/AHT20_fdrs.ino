//  FARM DATA RELAY SYSTEM
//
//  AHT20 SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.

#include "fdrs_sensor_config.h"
#include <Adafruit_AHTX0.h>
#include <fdrs_sensor.h>

Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  beginFDRS();
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
}

void loop() {  
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  loadFDRS(temp.temperature, TEMP_T);
  loadFDRS(humidity.relative_humidity, HUMIDITY_T);
  sendFDRS();
  sleepFDRS(60);  //Sleep time in seconds
}
