#define UNIT_MAC     0xFD  // THIS UNIT
#define ESPNOW1_MAC  0xFE  // ESPNOW1 Address 
#define ESPNOW2_MAC  0xFF  // ESPNOW2 Address

#define ESPNOW1_DELAY  0
#define ESPNOW2_DELAY  0
#define ESPNOWG_DELAY  0
#define SERIAL_DELAY   0
#define MQTT_DELAY     0
#define LORA_DELAY     0

#define ESPNOW1_ACT    
#define ESPNOW2_ACT    
#define ESPNOWG_ACT    
#define SERIAL_ACT     
#define MQTT_ACT       
#define LORA_ACT       

#define RXD2 21
#define TXD2 22

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

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE
