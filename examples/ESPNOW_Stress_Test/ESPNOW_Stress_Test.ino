//  FARM DATA RELAY SYSTEM
//
//  ESP-NOW Stress Tester or "Spammer"
//  
//  Sends ESP-NOW packets at approximately 60Hz.
//

#include "fdrs_node_config.h"
#include <fdrs_node.h>

void setup() {
  beginFDRS();
}
void loop() {
  for (uint8_t i=0; i < 255; i++) {
    loadFDRS(float(i), IT_T);
    sendFDRS();
    delay(15);
  }
}
