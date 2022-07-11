#include "fdrs_gateway_config.h"
#include "fdrs_gateway.h"

std::vector<DataReading_t> espnow_peer_unknown_data;
std::vector<DataReading_t> espnow_peer_1_data;
std::vector<DataReading_t> espnow_peer_2_data;

uint8_t selfAddress[6] =   {MAC_PREFIX, UNIT_MAC};
uint8_t ESPNOW1[] =       {MAC_PREFIX, ESPNOW_PEER_1};
uint8_t ESPNOW2[] =       {MAC_PREFIX, ESPNOW_PEER_2};

// ToDo: include global configuration values and use them here
//MQTT_FDRSGateWay MQTT(FDRS_WIFI_SSID,FDRS_WIFI_PASS,FDRS_MQTT_ADDR,FDRS_MQTT_PORT);
MQTT_FDRSGateWay MQTT(WIFI_SSID,WIFI_PASS,MQTT_ADDR,MQTT_PORT);
ESP_FDRSGateWay ESPNow;

void setup() {

  MQTT.init();

  ESPNow.init(selfAddress);
  ESPNow.add_peer(ESPNOW1);
  ESPNow.add_peer(ESPNOW2);

}

void loop() {

  espnow_peer_unknown_data = ESPNow.get_unkown_peer_data();
  espnow_peer_1_data = ESPNow.get_peer_data(ESPNOW1);
  espnow_peer_2_data = ESPNow.get_peer_data(ESPNOW2);

  MQTT.release(espnow_peer_unknown_data);
  MQTT.release(espnow_peer_1_data);
  MQTT.release(espnow_peer_2_data);

  ESPNow.flush();
  ESPNow.flush(ESPNOW1);
  ESPNow.flush(ESPNOW2);
  MQTT.flush();

  delay(1000);

}