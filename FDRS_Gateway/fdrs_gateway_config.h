//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

//#include <fdrs_globals.h> //Uncomment if you install the globals file
#define FDRS_DEBUG     //Enable USB-Serial debugging

#define UNIT_MAC     0x01  // The address of this gateway

//Actions -- Define what happens when a packet arrives at each interface:
// Current function options are: sendESPNOW(MAC), sendSerial(), sendMQTT(),
//  bufferLoRa(interface), bufferESPNOW(interface), bufferSerial(), and bufferMQTT().

#define ESPNOWG_ACT    sendSerial();
#define SERIAL_ACT         
#define MQTT_ACT          
#define LORAG_ACT      sendSerial();

//#define USE_LORA
//#define USE_WIFI      //Used only for MQTT gateway
//#define USE_SD_LOG      //Used only for SD-card logging
//#define USE_FS_LOG      //Used only for SPIFFS logging (esp internal filesystem)

// Peer addresses
#define ESPNOW1_PEER  0x0E  // ESPNOW1 Address 
#define ESPNOW2_PEER  0x0F  // ESPNOW2 Address
#define LORA1_PEER    0x0E  // LoRa1 Address
#define LORA2_PEER    0x0F  // LoRa2 Address

// Peer Actions
#define ESPNOW1_ACT    
#define ESPNOW2_ACT                    
#define LORA1_ACT 
#define LORA2_ACT 

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
#define LORA_BAND 915E6
#define LORA_SF 7

// Buffer Delays - in milliseconds - Uncomment to enable any buffer

//#define ESPNOW1_DELAY  0
//#define ESPNOW2_DELAY  0
//#define ESPNOWG_DELAY  0
//#define SERIAL_DELAY   0
//#define MQTT_DELAY     0
//#define LORAG_DELAY    1000
//#define LORA1_DELAY    1000
//#define LORA2_DELAY    1000

//#define USE_LED    //Not yet fully implemented
#define LED_PIN    32
#define NUM_LEDS    4

//WiFi and MQTT Credentials -- Needed only for MQTT gateway
#define WIFI_SSID   "Your SSID"  
#define WIFI_PASS   "Your Password"
#define MQTT_ADDR   "192.168.0.8"
#define MQTT_PORT   1883 // Default MQTT port is 1883

//MQTT Credentials -- Needed only if MQTT broker requires authentication
//#define MQTT_AUTH   //uncomment to enable MQTT authentication 
#define MQTT_USER   "Your MQTT Username"
#define MQTT_PASS   "Your MQTT Password"

// MQTT Topics
#define TOPIC_DATA "fdrs/data"
#define TOPIC_STATUS "fdrs/status"
#define TOPIC_COMMAND "fdrs/command" 


// Logging settings
#define LOGBUF_DELAY 10000      // Log Buffer Delay - in milliseconds               -- Needed only for SD-card OR internal flash logging

#define SD_SS 0     //SD card Chipselect pin (Use a different pins for LoRa and SD) -- Needed only for SD-card logging
#define SD_FILENAME "fdrs_log.csv"  // length max. 32                               -- Needed only for SD-card logging

#define FS_FILENAME "fdrs_log.csv"  // length max. 32                               -- Needed only for internal flash logging
