//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration
//  This is still in progress. Stay tuned!
#define RXD2 21
#define TXD2 22
#define UNIT_MAC 0x00// THIS UNIT
#define PREV_MAC 0x01// ESPNOW1 Address 
#define NEXT_MAC 0x02// ESPNOW2 Address

//#define USE_WIFI    //You cannot use ESP-NOW while WiFi is in use
//#define WIFI_NET "Your SSID"
//#define WIFI_PASS "Password"
//#define MQTT_ADDR "192.168.0.8"

//#define USE_LORA      
//#define SCK 5
//#define MISO 19
//#define MOSI 27
//#define SS 18
//#define RST 14
//#define DIO0 26
////433E6 for Asia
////866E6 for Europe
////915E6 for North America
//#define BAND 915E6


//Actions -- Define what happens when a packet arrives at each interface:
//Current function options are: sendESPNOW(interface),  sendSerial(), sendMQTT(), and sendLoRa().

#define ESPNOW1_ACT sendESPNOW(2); sendSerial();
#define ESPNOW2_ACT sendESPNOW(1); sendSerial();
#define ESPNOWG_ACT sendSerial();  sendLoRa();
#define SERIAL_ACT  sendESPNOW(0); sendLoRa();
#define MQTT_ACT    sendSerial();
#define LORA_ACT    sendSerial();

//#define ESPNOW1_ACTION
//#define ESPNOW2_ACTION
//#define ESPNOWG_ACTION
//#define SERIAL_ACTION   sendMQTT();
//#define MQTT_ACTION     sendSerial();
