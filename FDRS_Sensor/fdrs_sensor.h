//  FARM DATA RELAY SYSTEM
//
//  "fdrs_sensor.h"
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
#include <fdrs_datatypes.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#ifdef USE_LORA
#include <LoRa.h>
#endif

#ifdef FDRS_GLOBALS
#define FDRS_BAND GLOBAL_LORA_BAND
#define FDRS_SF GLOBAL_LORA_SF
#else
#define FDRS_BAND LORA_BAND
#define FDRS_SF LORA_SF
#endif

#ifdef FDRS_DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

enum {
  cmd_clear,
  cmd_ping,
  cmd_add,
};

typedef struct __attribute__((packed)) SystemPacket {
  uint8_t cmd;
  uint32_t param;
} SystemPacket;

const uint16_t espnow_size = 250 / sizeof(DataReading);
uint8_t gatewayAddress[] = {MAC_PREFIX, GTWY_MAC};
uint8_t gtwyAddress[] = {gatewayAddress[3], gatewayAddress[4], GTWY_MAC};
uint8_t LoRaAddress[] = {0x42, 0x00};

uint32_t wait_time = 0;
DataReading fdrsData[espnow_size];
uint8_t data_count = 0;
bool is_ping = false;

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
}
void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  if (len < sizeof(DataReading)) {
    SystemPacket command;
    memcpy(&command, incomingData, sizeof(command));
    if (command.cmd == cmd_ping) {
      is_ping = true;
      return;
    }
  }
}

void beginFDRS() {
#ifdef FDRS_DEBUG
  Serial.begin(115200);
#endif
  DBG("FDRS Sensor ID " + String(READING_ID) + " initializing...");
  DBG(" Gateway: " + String (GTWY_MAC, HEX));
#ifdef POWER_CTRL
  DBG("Powering up the sensor array!");
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
#endif
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#ifdef USE_ESPNOW
  DBG("Initializing ESP-NOW!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
#if defined(ESP8266)
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peers
  esp_now_add_peer(gatewayAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)

  if (esp_now_init() != ESP_OK) {
    DBG("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Register first peer
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer");
    return;
  }
#endif
  DBG(" ESP-NOW Initialized.");
#endif
#ifdef USE_LORA
  DBG("Initializing LoRa!");
  DBG(FDRS_BAND);
  DBG(FDRS_SF);
#ifdef ESP32
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
#endif
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(FDRS_BAND)) {
    DBG("Unable to initialize LoRa!");
    while (1);
  }
  LoRa.setSpreadingFactor(FDRS_SF);
  DBG(" LoRa Initialized.");
#endif
}
void transmitLoRa(uint8_t* mac, DataReading * packet, uint8_t len) {
#ifdef USE_LORA
  uint8_t pkt[5 + (len * sizeof(DataReading))];
  memcpy(&pkt, mac, 3);  //
  memcpy(&pkt[3], &LoRaAddress, 2);
  memcpy(&pkt[5], packet, len * sizeof(DataReading));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
#endif
}
void sendFDRS() {
  DBG("Sending FDRS Packet!");
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *) &fdrsData, data_count * sizeof(DataReading));
  delay(5);
  DBG(" ESP-NOW sent.");
#endif
#ifdef USE_LORA
  transmitLoRa(gtwyAddress, fdrsData, data_count);
  DBG(" LoRa sent.");
#endif
  data_count = 0;
}
void loadFDRS(float d, uint8_t t) {
  DBG("Data loaded. Type: " + String(t));
  if (data_count > espnow_size) sendFDRS();
  DataReading dr;
  dr.id = READING_ID;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}
void sleepFDRS(int sleep_time) {
  DBG("Sleepytime!");
#ifdef DEEP_SLEEP
  DBG(" Deep sleeping.");
#ifdef ESP32
  esp_sleep_enable_timer_wakeup(sleep_time * 1000000);
  esp_deep_sleep_start();
#endif
#ifdef ESP8266
  ESP.deepSleep(sleep_time * 1000000);
#endif
#endif
  DBG(" Delaying.");
  delay(sleep_time * 1000);
}

void pingFDRS(int timeout) {
  SystemPacket sys_packet;
  sys_packet.cmd = cmd_ping;
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *) &sys_packet, sizeof(SystemPacket));
  DBG(" ESP-NOW ping sent.");
  uint32_t ping_start = millis();
  is_ping = false;
  while ((millis() - ping_start) <= timeout) {
    yield(); //do I need to yield or does it automatically?
    if (is_ping) {
      DBG("Ping Returned:" + String(millis() - ping_start));
      break;
    }
  }

#endif
#ifdef USE_LORA
  //transmitLoRa(gtwyAddress, sys_packet, data_count); // TODO: Make this congruent to esp_now_send()
  DBG(" LoRa ping not sent because it isn't implemented.");
#endif
}
