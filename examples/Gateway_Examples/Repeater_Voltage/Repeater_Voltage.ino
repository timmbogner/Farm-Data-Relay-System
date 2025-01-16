#define GTWY_READING_ID 42
#define INTERVAL_SECONDS 60
#define ADC_PIN 34

#include "fdrs_gateway_config.h"
#include <fdrs_gateway.h>
int vref = 1100;

float getVoltage() {
  uint16_t v = analogRead(ADC_PIN);
  float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  String voltage = "Voltage :" + String(battery_voltage) + "V";
  DBG(voltage);
  return battery_voltage;
}
void sendReading() {
  float v = getVoltage();
  loadFDRS(v, VOLTAGE_T, GTWY_READING_ID);
  sendFDRS();
}

void setup() {
  beginFDRS();
  sendReading();
  scheduleFDRS(sendReading, INTERVAL_SECONDS * 1000);
  sendReading();
}

void loop() {
  loopFDRS();
}