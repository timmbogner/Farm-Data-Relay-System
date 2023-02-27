//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//

#include "fdrs_gateway_config.h"
#include <fdrs_gateway.h>

time_t lastRunTime = 0;
uint8_t LoRaAddress[] = { 0xFF, 0xFF };

void setup() {
beginFDRS();
}

void loop() {
    loopFDRS();
    // Send time to LoRa broadcast address every 30 seconds.
    // LoRa controllers should receive time and report via serial
    if(millis() - lastRunTime > (1000 * 30)) {
        DBG("Sending LoRa time.");
        timeFDRSLoRa(LoRaAddress);
        lastRunTime = millis();
    }
}