//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

//#include <fdrs_globals.h> //Uncomment if you install the globals file
#define DEBUG

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

#define UNIT_MAC     0x03  // The address of this gateway

//Actions -- Define what happens when a packet arrives at each interface:
//Current function options are: sendESPNOW(MAC), sendSerial(), sendMQTT(), bufferESPNOW(interface), bufferSerial(), and bufferLoRa(interface).

#define SERIAL_ACT         
#define MQTT_ACT          
#define LORAG_ACT      

//#define USE_LORA
//#define USE_WIFI    //Used only for MQTT gateway

// Peer addresses

#define LORA_PEER_1    0x0E  // LoRa1 Address
#define LORA_PEER_2    0x0F  // LoRa2 Address

#define ESPNOW_PEER_1  0x0C  // ESPNOW1 Address 
#define ESPNOW_PEER_2  0x0D  // ESPNOW2 Address

// Peer Actions
#define ESPNOW1_ACT    
#define ESPNOW2_ACT                    
#define LORA1_ACT 
#define LORA2_ACT 

//WiFi and MQTT Credentials -- Needed only for MQTT gateway
#define WIFI_SSID   "Your SSID"  
#define WIFI_PASS   "Your Password"
#define MQTT_ADDR   "192.168.0.8"
#define MQTT_PORT   1883

//Pins for UART data interface (ESP32 only)
#define RXD2 14
#define TXD2 15

//LoRa Configuration -- Needed only if using LoRa
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
#define SF 7

//#define USE_LED    //Not yet fully implemented
#define LED_PIN    32
#define NUM_LEDS    4
