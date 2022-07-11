/*  FARM DATA RELAY SYSTEM
*
*  "fdrs_sensor.h"
*
*  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
*  Condensed and refactored to a single file pair by Binder Tronics (info@bindertronics.com).
*/

#ifndef __FDRS_GATEWAY_H__
#define __FDRS_GATEWAY_H__

#include "HardwareSerial.h"
#include "fdrs_types.h"
#include <string.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "LoRa.h"


#define USE_LORA

#define FDRS_DEBUG

#ifdef FDRS_DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif


class FDRSGateWayBase{
public:

    FDRSGateWayBase();
    ~FDRSGateWayBase();

    void release(std::vector<DataReading_t> data, uint8_t *peer_mac = NULL);
    virtual void flush(uint8_t *peer_mac = NULL) = 0;

private:
    static uint32_t peer_id;
    virtual void send(std::vector<DataReading_t> data) = 0;
    virtual void forward(uint8_t *peer_mac ,std::vector<DataReading_t> data) = 0;
};

class ESP_FDRSGateWay: public FDRSGateWayBase{
public:
    ESP_FDRSGateWay(void);

    static void OnDataRecv(uint8_t * mac, const uint8_t *incomingData, int len);

    void init(uint8_t inturnal_mac[5]);

    void add_peer(uint8_t peer_mac[6]);
    void remove_peer(uint8_t peer_mac[6]);
    std::vector<DataReading_t> get_peer_data(uint8_t peer_mac[6]);
    std::vector<DataReading_t> get_unkown_peer_data(void);
    
    void flush(uint8_t *peer_mac = NULL) override;

private:

    static bool is_init;
    uint8_t _broadcast_mac[6];
    uint8_t _inturnal_mac[6];
    static std::vector<Peer_t> _peer_list;
    static std::vector<Peer_t> _unknow_peer;

    static void setup(void);

    void send(std::vector<DataReading_t> data) override;
    void forward(uint8_t *peer_mac ,std::vector<DataReading_t> data) override;

    void list_peer(uint8_t peer_mac[6]);
    void unlist_peer(uint8_t peer_mac[6]);

};


class MQTT_FDRSGateWay: public FDRSGateWayBase{

public:

    MQTT_FDRSGateWay(const char *ssid, const char *password, const char *server,int port = 1883);
    ~MQTT_FDRSGateWay(void);

    void init(void);

    std::vector<DataReading_t> get_data(void);

    void flush(uint8_t *peer_mac = NULL) override;

private:
    #define TOPIC_DATA "fdrs/data"
    #define TOPIC_STATUS "fdrs/status"
    #define TOPIC_COMMAND "fdrs/command"

    static void mqtt_callback(char* topic, byte * message, unsigned int length);

    const char *_ssid;
    const char *_password;
    const char *_server;
    int _port;
    WiFiClient espClient;
    PubSubClient *_client;

    static std::vector<DataReading_t> _buffer;

    void reconnect();
    void send(std::vector<DataReading_t> data) override;
    void forward(uint8_t *peer_mac ,std::vector<DataReading_t> data) override;

};


class Serial_FDRSGateWay: public FDRSGateWayBase{

public:
    Serial_FDRSGateWay(HardwareSerial *serial, uint32_t baud);

    void init(void);
#if defined(ESP32)
    void init(int mode, int rx_pin, int tx_pin);
#endif
    void get(void);

    std::vector<DataReading_t> get_data(void);

    void flush(uint8_t *peer_mac = NULL) override;

private:

    HardwareSerial *_serial;
    uint32_t _baud;
    static std::vector<DataReading_t> _buffer;

    static void setup(void);
    void pull(void);
    void send(std::vector<DataReading_t> data) override;
    void forward(uint8_t *peer_mac ,std::vector<DataReading_t> data) override;
    
};


class LoRa_FDRSGateWay: public FDRSGateWayBase{

public:
    LoRa_FDRSGateWay(uint8_t miso,uint8_t mosi,uint8_t sck, uint8_t ss,uint8_t rst,uint8_t dio0,double band,uint8_t sf);

    void init(uint8_t mac[6]);
    void get(void);
    void add_peer(uint8_t peer_mac[6]);
    void remove_peer(uint8_t peer_mac[6]);
    std::vector<DataReading_t> get_peer_data(uint8_t *peer_mac);

    void flush(uint8_t *peer_mac = NULL) override;

private:

    uint8_t _mac[6];
    uint8_t _miso;
    uint8_t _mosi;
    uint8_t _sck;
    uint8_t _ss;
    uint8_t _rst;
    uint8_t _dio0;
    uint32_t _band;
    uint8_t _sf;

    std::vector<Peer_t> _peer_list;

    void transmit(DataReading_t *packet, uint8_t len);

    void send(std::vector<DataReading_t> data) override;
    void forward(uint8_t *peer_mac ,std::vector<DataReading_t> data) override;

};



#endif