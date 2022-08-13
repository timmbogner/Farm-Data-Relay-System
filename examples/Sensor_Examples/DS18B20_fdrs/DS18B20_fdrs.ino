//  FARM DATA RELAY SYSTEM
//
//  DS18B20 SENSOR MODULE
//

#define ONE_WIRE_BUS  13   //Pin that the DS18B20 is connected to

#include "fdrs_sensor_config.h"
#include <fdrs_sensor.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  beginFDRS();
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0);
  loadFDRS(tempC, TEMP_T);
  sendFDRS();
  sleepFDRS(60);
}
