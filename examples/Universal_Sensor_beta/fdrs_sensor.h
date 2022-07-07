/*  FARM DATA RELAY SYSTEM
*
*  "fdrs_sensor.h"
*
*  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
*  Condensed and refactored to a single file pair by Binder Tronics (info@bindertronics.com).
*/

#ifndef  __FDRS_SENSOR__H__
#define __FDRS_SENSOR__H__

#include "fdrs_types.h"
#include "fdrs_datatypes.h"

//1 to enable debugging prints. 0 disables the debugging prints
#define ENABLE_DEBUG 1 

#if ENABLE_DEBUG == 1
#ifndef DEBUG
#define DEBUG
#endif
#endif

#define USE_LORA

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif

#ifdef USE_LORA
#include "LoRa.h"
#endif

#ifdef GLOBALS
#define FDRS_BAND GLOBAL_BAND
#define FDRS_SF GLOBAL_SF
#else
#define FDRS_BAND BAND
#define FDRS_SF SF
#endif

#ifdef DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

extern const uint8_t prefix[5];

#define MAC_PREFIX prefix   // Should only be changed if implementing multiple FDRS systems.

#define ESP_GATEWAY_ADDRESS_SIZE 6
#define LORA_GATEWAY_ADDRESS_SIZE 3

class FDRSBase{
public:

  FDRSBase(uint8_t gtwy_mac,uint8_t reading_id);
  ~FDRSBase();

  void begin(void);
  void load(float data, uint8_t type);
  void sleep(int seconds);
  void send();

private:

  uint8_t _gtwy_mac;
  const uint16_t _espnow_size;
  uint8_t _reading_id;
  uint8_t _data_count;
  DataReading_t *fdrsData;

  virtual void init(void) = 0;
  virtual void transmit(DataReading_t *fdrsData, uint8_t _data_count) = 0;
};


class FDRS_EspNow: public FDRSBase{
public:

  FDRS_EspNow(uint8_t gtwy_mac, uint8_t reading_id);

private:

  uint8_t _gatewayAddress[ESP_GATEWAY_ADDRESS_SIZE];
  void transmit(DataReading_t *fdrsData, uint8_t _data_count) override;
  void init(void) override;

};

class FDRSLoRa: public FDRSBase{
public:

  FDRSLoRa(uint8_t gtwy_mac, uint8_t reading_id,uint8_t miso,uint8_t mosi,uint8_t sck, uint8_t ss,uint8_t rst,uint8_t dio0,uint32_t band,uint8_t sf);

private:

  uint8_t _gatewayAddress[LORA_GATEWAY_ADDRESS_SIZE];
  uint8_t _miso;
  uint8_t _mosi;
  uint8_t _sck;
  uint8_t _ss;
  uint8_t _rst;
  uint8_t _dio0;
  uint32_t _band;
  uint8_t _sf;

  void buildPacket(uint8_t* mac, DataReading_t * packet, uint8_t len);
  void transmit(DataReading_t *fdrsData, uint8_t _data_count) override;
  void init(void) override;

};

#endif