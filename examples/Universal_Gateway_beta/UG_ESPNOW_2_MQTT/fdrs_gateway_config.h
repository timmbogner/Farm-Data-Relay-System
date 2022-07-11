//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

#include <fdrs_globals.h> // uncomment if you want to set specific values for this sensor in sensor_setup.h
#define FDRS_DEBUG

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

#define UNIT_MAC     0x03  // The address of this gateway

//Where we get the data from
#define LORA_GET
#define MQTT_GET
#define ESP_GET
#define SER_GET  

//Where we send the data to
#define LORA_SEND
#define MQTT_SEND
#define ESP_SEND
#define SER_SEND 

//#define USE_LORA
//#define USE_WIFI    //Used only for MQTT gateway

// Peer addresses

#define LORA_PEER_1    0x0E  // LoRa1 Address
#define LORA_PEER_2    0x0F  // LoRa2 Address

#define ESPNOW_ALL           //send to all know and unknow peers
#define ESPNOW_PEER_1  0x0C  // ESPNOW1 Address 
#define ESPNOW_PEER_2  0x0D  // ESPNOW2 Address

// TODO: Needs to be commented out if FDRS_GLOBALS are assigned
//WiFi and MQTT Credentials -- Needed only for MQTT gateway
#define WIFI_SSID   "Your SSID"  
#define WIFI_PASS   "Your Password"
#define MQTT_ADDR   "192.168.0.8"
#define MQTT_PORT   1883

//Pins for UART data interface (ESP32 only)
#define RXD2 14
#define TXD2 15

//SPI Configuration -- Needed only on Boards with multiple SPI interfaces like the ESP32
#define SPI_SCK 5
#define SPI_MISO 19
#define SPI_MOSI 27

//LoRa Configuration -- Needed only if using LoRa
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
// TODO: Needs to be commented out if FDRS_GLOBALS are assigned
#define LORA_BAND 866E6
#define LORA_SF 7

//#define USE_LED    //Not yet fully implemented
#define LED_PIN    32
#define NUM_LEDS    4
