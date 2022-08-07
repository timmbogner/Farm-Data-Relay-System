//  FARM DATA RELAY SYSTEM
//
//  Multifunction ESP8266 Sensor Board by Phil Grant
//
//  https://github.com/gadjet/Multifunction-ESP8266-Sensor-board
//  

#include "fdrs_sensor_config.h"
#include <Adafruit_AHT10.h>
#include <fdrs_sensor.h>

Adafruit_AHT10 aht;

const int reedSwitch = 13;

void setup() {
  aht.begin();

  // Init Serial Monitor
  //Serial.begin(115200);
  // initialize the reed switch pin as an input:
  pinMode(reedSwitch, INPUT);
  // initialize the wakeup pin as an input:
  pinMode(16, WAKEUP_PULLUP);
  beginFDRS();
}


void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  // Read the state of the reed switch and send open or closed
  if (digitalRead(reedSwitch) == HIGH) {
    loadFDRS(1.0, MOTION_T);
  }
  else {
    loadFDRS(0.0, MOTION_T);
  }
  loadFDRS((analogRead(A0) * 4.2 * 10 / 1023), VOLTAGE_T);
  loadFDRS(humidity.relative_humidity, HUMIDITY_T);
  loadFDRS(temp.temperature, TEMP_T);
  // Send message via FDRS
  sendFDRS();
  sleepFDRS(15);   //15 Min's sleep
}
