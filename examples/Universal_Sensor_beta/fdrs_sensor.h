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
#include <fdrs_datatypes.h>
#include "fdrs_sensor_config.h"

//1 to enable debugging prints. 0 disables the debugging prints
#define ENABLE_DEBUG 1 

#if ENABLE_DEBUG == 1
#ifndef FDRS_DEBUG
#define FDRS_DEBUG
#endif
#endif

#define USE_LORA
//#define USE_ESPNOW

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

//#ifdef FDRS_GLOBAL_LORA
#define FDRS_BAND GLOBAL_LORA_BAND
#define FDRS_SF GLOBAL_LORA_SF
#define FDRS_TXPWR GLOBAL_FDRS_TXPWR
#else
#define FDRS_BAND LORA_BAND
#define FDRS_SF LORA_SF
#define FDRS_TXPWR LORA_TXPWR
#endif //FDRS_GLOBAL_LORA

#ifdef FDRS_DEBUG
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
 /**
  * @brief Construct a new FDRSBase object
  * 
  * @param gtwy_mac     The network MAC
  * @param reading_id   The identifier for the sensor
  */

  FDRSBase(uint8_t gtwy_mac,uint16_t reading_id);
  ~FDRSBase();

  /**
   * @brief Start a paticulare sensor.
   */
  void begin(void);

  /**
   * @brief Set the data for the next transmission.
   * 
   * @param data Data for the next transmission
   * @param type The ID for the type of data that is being sent.
   */
  void load(float data, uint8_t type);

  /**
   * @brief Time in seconds the sensor will delay before sending data.
   * 
   * @note This is blocking.
   * 
   * @param seconds Number of seconds to delay.
   */
  void sleep(int seconds);

  /**
   * @brief  Send the data out on the network.
   * 
   */
  void send();

private:

  uint8_t _gtwy_mac;
  const uint16_t _espnow_size;
  uint16_t _reading_id;
  uint8_t _data_count;
  DataReading_t *fdrsData;

  /**
   * @brief Required implementation of a paticulare sensors initialization.
   */
  virtual void init(void) = 0;

  /**
   * @brief Required implementation of how a sensor will send its data out on the network.
   * 
   * @param fdrsData    Pointer to the data that the sensor will be seding.
   * @param _data_count The number of data packets the sensor will be sending.
   */
  virtual void transmit(DataReading_t *fdrsData, uint8_t _data_count) = 0;
};


class FDRS_EspNow: public FDRSBase{
public:

  /**
   * @brief Construct a new fdrs espnow object
   * 
   * @param gtwy_mac    The network MAC.
   * @param reading_id  The identifier for the sensor.
   */

  FDRS_EspNow(uint8_t gtwy_mac, uint16_t reading_id);

private:

  uint8_t _gatewayAddress[ESP_GATEWAY_ADDRESS_SIZE];

  /**
   * @brief Send data out on the ESPNOW network.
   * 
   * @param fdrsData    Pointer to the data that the sensor will be seding.
   * @param _data_count The number of data packets the sensor will be sending.
   */
  void transmit(DataReading_t *fdrsData, uint8_t _data_count) override;

  /**
   * @brief Initialize the ESPNOW network
   * 
   */
  void init(void) override;

};

class FDRSLoRa: public FDRSBase{
public:

  /**
   * @brief Construct a new FDRSLoRa object
   * 
   * @param gtwy_mac    The network MAC.
   * @param reading_id  The identifier for the sensor.
   * @param miso        Master in slave out pin.
   * @param mosi        Master out slave in pin.
   * @param sck         Master clock pin.
   * @param ss          Slave select pin
   * @param rst         Reset Pin 
   * @param dio0        LoRa data pin
   * @param band        LoRa frequency band
   * @param sf          LoRa spread factor
   */

  FDRSLoRa(uint8_t gtwy_mac, uint16_t reading_id,uint8_t miso,uint8_t mosi,uint8_t sck, uint8_t ss,uint8_t rst,uint8_t dio0,uint32_t band,uint8_t sf);

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

  /**
   * @brief Construct a LoRa packet
   * 
   * @param mac     Network Mac address
   * @param packet  Data to create the packet with
   * @param len     Length of the data in bytes
   */
  void buildPacket(uint8_t* mac, DataReading_t * packet, uint8_t len);

  /**
   * @brief Send data out on the LoRa network.
   * 
   * @param fdrsData    Pointer to the data that the sensor will be seding.
   * @param _data_count The number of data packets the sensor will be sending.
   */
  void transmit(DataReading_t *fdrsData, uint8_t _data_count) override;

  /**
   * @brief Initialize the LoRa module 
   */
  void init(void) override;

};

#endif
