//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

#include "defaults.h"

#define UNIT_MAC     0x01  // THIS UNIT

//Actions -- Define what happens when a packet arrives at each interface:
//Current function options are: sendESPNOW(MAC), sendSerial(), sendMQTT(), bufferESPNOW(interface), bufferSerial(), and bufferLoRa(interface).
  
#define ESPNOWG_ACT    sendSerial();
#define SERIAL_ACT     
#define MQTT_ACT       sendSerial();   
#define LORAG_ACT      sendSerial();
 
 
//ESP32 Only
//#define RXD2 21
//#define TXD2 22

//#define USE_WIFI    //  You should not use ESP-NOW while WiFi is in use.
#define WIFI_NET "Your SSID"
#define WIFI_PASS "Password"
#define MQTT_ADDR "192.168.0.8"

//#define USE_LORA      
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6
