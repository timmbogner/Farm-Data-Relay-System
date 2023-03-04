//  FARM DATA RELAY SYSTEM
//
//  ETHERNET GATEWAY
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.

//  Configuration for the ThingPulse ESPGateway: https://thingpulse.com/product/espgateway-ethernet-esp32-wifi-ble-gateway-with-rj45-ethernet-connector/
#define ETH_CLK_MODE ETH_CLOCK_GPIO16_OUT
#define ETH_POWER_PIN 5

#include "fdrs_gateway_config.h"
#include <fdrs_gateway.h>

void setup() {
  pinMode(ETH_POWER_PIN, OUTPUT);
  digitalWrite(ETH_POWER_PIN, HIGH);
  beginFDRS();
}

void loop() {
  loopFDRS();
}
