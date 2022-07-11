//  FARM DATA RELAY SYSTEM
//
//  ESP-NOW Stress Tester or "Spammer"
//  
//  Sends ESP-NOW packets at approximately 60Hz.
//

#include "fdrs_sensor_config.h"
#include <fdrs_sensor.h>

void setup() {
  beginFDRS();
}
void loop() {
  for (int i=0; i < 255; i++) {
    loadFDRS(float(i), IT_T);
    sendFDRS();
    delay(15);
  }
}
