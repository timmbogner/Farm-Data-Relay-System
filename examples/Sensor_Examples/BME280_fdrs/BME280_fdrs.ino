//  FARM DATA RELAY SYSTEM
//
//  BME280 SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.

#include "fdrs_sensor_config.h"
#include <Adafruit_BME280.h>
#include <fdrs_sensor.h>

Adafruit_BME280 bme;

void setup() {
  //Serial.begin(115200);
  beginFDRS();
  while (!bme.begin(0x76)) {
    //Serial.println("BME not initializing!");
    delay(10);
  }
}

void loop() {
  loadFDRS(bme.readTemperature(), TEMP_T);
  loadFDRS(bme.readHumidity(), HUMIDITY_T);
  loadFDRS(bme.readPressure() / 100.0F, PRESSURE_T);
  sendFDRS();
  sleepFDRS(60);  //Sleep time in seconds
}
