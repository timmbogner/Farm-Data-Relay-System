//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

#include "defaults.h"
#define UNIT_MAC     0x00  // THIS UNIT

//Actions -- Define what happens when a packet arrives at each interface:
//Current function options are: sendESPNOW(interface),  sendSerial(), sendMQTT(), and sendLoRa().

#define ESPNOWG_ACT    sendSerial();

//ESP32 Only
#define RXD2 21
#define TXD2 22

//#define USE_WIFI    //You should not use ESP-NOW while WiFi is in use
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

#define ESPNOW1_DELAY  0
#define ESPNOW2_DELAY  0
#define ESPNOWG_DELAY  0
#define SERIAL_DELAY   0
#define MQTT_DELAY     0
#define LORA_DELAY     0

//Use these settings for a gateway that recieves ESP-NOW data and sends serial (UART).
//#define ESPNOW1_ACT    sendSerial();
//#define ESPNOW2_ACT    sendSerial();
//#define ESPNOWG_ACT    sendSerial();
//#define SERIAL_ACT     sendESPNOW(0);      //sendESPNOW() routine will soon change
//#define MQTT_ACT       
//#define LORA_ACT       sendSerial();

//Use these settings for a gateway that recieves serial (UART) data and sends MQTT.
//#define USE_WIFI
//#define SERIAL_ACT   sendMQTT();

//Use these settings for a basic repeater addressed to the final gateway.  
//#define UNIT_MAC     0x01  // THIS UNIT
//#define ESPNOW1_MAC  0x00  // ESPNOW1 Address 
//#define ESPNOW2_MAC  0x02  // ESPNOW2 Address
//#define ESPNOW1_ACT  sendESPNOW(2);
//#define ESPNOW2_ACT  sendESPNOW(1);         //sendESPNOW() routine will soon change
//#define ESPNOWG_ACT  sendESPNOW(1);
   
