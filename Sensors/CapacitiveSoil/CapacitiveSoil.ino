//  FARM DATA RELAY SYSTEM
//
//  CAPACITIVE SOIL MOISTURE SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Connect sensor to the [first] analog pin of the ESP (A0).
//
#define CALIB_H 285.0f  //High and Low calibrations
//#define CALIB_L 485.0f
#define CALIB_L 350.0f

#include "fdrs_sensor.h"

void setup() {
  Serial.begin(115200);
  beginFDRS();
}
void loop() {
  uint16_t s = analogRead(0);
  float r;
  if (s < CALIB_H) s = CALIB_H;
  if (s > CALIB_L) s = CALIB_L;
  r = map(float(s), CALIB_H, CALIB_L, 100.0f, 0.0f);
  Serial.println(s);
  Serial.println(r);

  loadFDRS(r, SOIL_T);
  sendFDRS();
  sleepFDRS(60);
}
