//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

#include "defaults.h"

#define UNIT_MAC     0x00  // The address of this gateway

//Actions -- Define what happens when a packet arrives at each interface:
//Current function options are: sendESPNOW(MAC), sendSerial(), sendMQTT(), bufferESPNOW(interface), bufferSerial(), and bufferLoRa(interface).
  
#define ESPNOWG_ACT    sendSerial();
#define SERIAL_ACT     
#define MQTT_ACT          
#define LORAG_ACT      sendSerial();

#define USE_LORA      
//#define USE_WIFI    //Used only for MQTT gateway

#define CREDENTIALS 

#if defined (ESP32)
#define RXD2 14
#define TXD2 15
#define UART_IF Serial2
#else 
#define UART_IF Serial
#endif

//LoRa Configuration -- Needed only if this device is using LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// default.h values are overwritten by the #defines of credentials.h files in the library folder
#ifdef CREDENTIALS
#include <credentials.h>
#define WIFI_NET mySSID
#define WIFI_PASS myPASSWORD
#define MQTT_ADDR MQTT_BROKER
#define BAND myBAND
#define SF mySF
#endif
