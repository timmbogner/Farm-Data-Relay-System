//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

#include <fdrs_globals.h>
#define FDRS_DEBUG     //Enable USB-Serial debugging

#define UNIT_MAC     0x02  // The address of this gateway

// Actions -- Define what happens when a packet arrives at each interface:
// Current function options are: sendESPNOW(MAC), sendSerial(), sendMQTT(),
//  bufferLoRa(interface), bufferESPNOW(interface), bufferSerial(), and bufferMQTT().

#define ESPNOWG_ACT    sendESPNOW(0x01);
#define SERIAL_ACT         
#define MQTT_ACT          
#define LORAG_ACT      

// Protocols -- Define which protocols the gateway will use.
// Warning: ESP-NOW and WiFi should not be used simultaneously.

#define USE_ESPNOW  
//#define USE_LORA
//#define USE_WIFI    //Used only for MQTT gateway

// Peer addresses
#define ESPNOW1_PEER  0x01  // ESPNOW1 Address 
#define ESPNOW2_PEER  0x0F  // ESPNOW2 Address
#define LORA1_PEER    0x0E  // LoRa1 Address
#define LORA2_PEER    0x0F  // LoRa2 Address

// Peer Actions
#define ESPNOW1_ACT  sendSerial(); // This would display packets arriving *from* the front-end.    
#define ESPNOW2_ACT                    
#define LORA1_ACT 
#define LORA2_ACT 

//Pins for UART data interface (ESP32 only)
#define RXD2 14
#define TXD2 15

//Logging settings  --  Logging will occur when MQTT is disconnected
//#define USE_SD_LOG        //Enable SD-card logging
//#define USE_FS_LOG        //Enable filesystem (flash) logging
#define LOGBUF_DELAY 10000  // Log Buffer Delay - in milliseconds
#define SD_SS 0             //SD card CS pin (Use different pins for LoRa and SD)
#define SD_FILENAME "fdrs_log.csv"
#define FS_FILENAME "fdrs_log.csv"

// SPI Configuration -- Needed only on Boards with multiple SPI interfaces like the ESP32

#define SPI_SCK 5
#define SPI_MISO 19
#define SPI_MOSI 27

// LoRa Configuration -- Needed only if using LoRa
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define LORA_BAND 915E6     // LoRa Frequency Band
#define LORA_SF 7           // LoRa Spreading Factor
#define LORA_TXPWR 17       // LoRa TX power in dBm (+2dBm - +20dBm), default is +17dBm.

// Buffer Delays - in milliseconds - Uncomment to enable any buffer

//#define ESPNOW1_DELAY  0
//#define ESPNOW2_DELAY  0
//#define ESPNOWG_DELAY  0
//#define SERIAL_DELAY   0
//#define MQTT_DELAY     0
//#define LORAG_DELAY    1000
//#define LORA1_DELAY    1000
//#define LORA2_DELAY    1000

// FastLED -- Not yet fully implemented
//#define USE_LED
#define LED_PIN    32
#define NUM_LEDS    4

// WiFi and MQTT Credentials -- Needed for MQTT only if "fdrs_globals.h" is not included
//#define WIFI_SSID   "Your SSID"  
//#define WIFI_PASS   "Your Password"
//#define MQTT_ADDR   "192.168.0.8"
//#define MQTT_PORT   1883 // Default MQTT port is 1883

//#define MQTT_AUTH   //Enable MQTT authentication 
//#define MQTT_USER   "Your MQTT Username"
//#define MQTT_PASS   "Your MQTT Password"

// MQTT Topics
#define TOPIC_DATA "fdrs/data"
#define TOPIC_STATUS "fdrs/status"
#define TOPIC_COMMAND "fdrs/command" 
