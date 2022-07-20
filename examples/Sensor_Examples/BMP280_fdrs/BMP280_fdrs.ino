//  FARM DATA RELAY SYSTEM
//
//  BMP280 SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  Connect sensor SDA and SCL pins to those of the ESP.

#include "fdrs_sensor_config.h"
#include <Adafruit_BMP280.h>
#include <fdrs_sensor.h>

Adafruit_BMP280 bmp;

void setup() {
  //Serial.begin(115200);
  beginFDRS();
  while (!bmp.begin(0x76)) {
    //Serial.println("BMP not initializing!");
    delay(10);
  }
}

void loop() {
  loadFDRS(bmp.readTemperature(), TEMP_T);
  loadFDRS(bmp.readPressure() / 100.0F, PRESSURE_T);
  sendFDRS();
  sleepFDRS(60);  //Sleep time in seconds
}
