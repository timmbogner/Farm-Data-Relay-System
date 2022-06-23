#ifndef __FDRS_GATEWAY_H__
#define __FDRS_GATEWAY_H__

#include "fdrs_types.h"
#include <string.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "ArduinoJson.h"

#ifdef DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#if defined (ESP32)
#define UART_IF Serial1
#else
#define UART_IF Serial
#endif

#ifdef GLOBALS
#define FDRS_WIFI_SSID GLOBAL_SSID
#define FDRS_WIFI_PASS GLOBAL_PASS
#define FDRS_MQTT_ADDR GLOBAL_MQTT_ADDR
#define FDRS_BAND GLOBAL_BAND
#define FDRS_SF GLOBAL_SF
#else
#define FDRS_WIFI_SSID WIFI_SSID
#define FDRS_WIFI_PASS WIFI_PASS
#define FDRS_MQTT_ADDR MQTT_ADDR
#define FDRS_BAND BAND
#define FDRS_SF SF
#endif

#define USE_LORA

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.



const uint8_t espnow_size = 250 / sizeof(DataReading_t);
const uint8_t lora_size   = 256 / sizeof(DataReading_t);
// const uint8_t mac_prefix[] = {MAC_PREFIX};

// uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t selfAddress[] =   {MAC_PREFIX, UNIT_MAC};
// uint8_t incMAC[6];

// #ifdef ESPNOW1_PEER
// uint8_t ESPNOW1[] =       {MAC_PREFIX, ESPNOW1_PEER};
// #else
// uint8_t ESPNOW1[] =       {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// #endif
// #ifdef ESPNOW2_PEER
// uint8_t ESPNOW2[] =       {MAC_PREFIX, ESPNOW2_PEER};
// #else
// uint8_t ESPNOW2[] =       {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// #endif

// #ifdef USE_LORA
// uint8_t LoRa1[] =         {mac_prefix[3], mac_prefix[4], LORA1_PEER};
// uint8_t LoRa2[] =         {mac_prefix[3], mac_prefix[4], LORA2_PEER};
// //uint8_t LoRaAddress[] = {0x42, 0x00};
// #endif


//WiFiClient espClient;
#ifdef USE_LED
CRGB leds[NUM_LEDS];
#endif
#ifdef USE_WIFI
PubSubClient client(espClient);
const char* ssid = FDRS_WIFI_SSID;
const char* password = FDRS_WIFI_PASS;
const char* mqtt_server = FDRS_MQTT_ADDR;
#endif

void getSerial(void);

void mqtt_callback(char* topic, byte * message, unsigned int length);

void getLoRa();

void sendESPNOW(uint8_t address);

void sendSerial();

void sendMQTT();

void bufferESPNOW(uint8_t interface);

void bufferSerial();

void bufferLoRa(uint8_t interface);

void releaseESPNOW(uint8_t interface);

void transmitLoRa(uint8_t* mac, DataReading_t * packet, uint8_t len);

void releaseLoRa(uint8_t interface);

void releaseSerial();

void releaseMQTT();

void reconnect();

void begin_espnow();


class FDRSGateWayBase{
public:

    FDRSGateWayBase(uint32_t send_delay);
    ~FDRSGateWayBase();

    static void add_data(DataReading_t *data);

    void release(void);

private:
    uint32_t _send_delay;
    static uint32_t peer_id;
    static std::vector<DataReading_t> _data;
    static std::vector<FDRSGateWayBase*> _object_list;

    virtual void send(std::vector<DataReading_t> data) = 0;
};

class ESP_FDRSGateWay: public FDRSGateWayBase{
public:
    ESP_FDRSGateWay(uint8_t broadcast_mac[6],uint8_t inturnal_mac[5], uint32_t send_delay);

    void init(void);

    void add_peer(uint8_t peer_mac[6]);
    void remove_peer(uint8_t peer_mac[6]);
    static void OnDataRecv(uint8_t * mac, const uint8_t *incomingData, int len);

private:

    static bool is_init;
    uint32_t _send_delay;
    uint8_t _broadcast_mac[6];
    uint8_t _inturnal_mac[6];
    static std::vector<ESP_Peer_t> peer_list;
    static std::vector<ESP_Peer_t> unknow_peer;

    static void setup(void);

    virtual void send(std::vector<DataReading_t> data) override;

    void list_peer(uint8_t peer_mac[6]);
    void unlist_peer(uint8_t peer_mac[6]);

};

#endif