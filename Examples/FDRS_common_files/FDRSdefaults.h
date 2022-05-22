#ifdef DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#define UNIT_MAC     0xFC  // THIS UNIT
#define ESPNOW1_PEER  0xFD  // ESPNOW1 Address 
#define ESPNOW2_PEER  0xFE  // ESPNOW2 Address
#define LORA1_PEER    0xFD  // LoRa1 Address
#define LORA2_PEER    0xFE  // LoRa2 Address

#define ESPNOW1_DELAY  0
#define ESPNOW2_DELAY  0
#define ESPNOWG_DELAY  0
#define SERIAL_DELAY   0
#define MQTT_DELAY     0
#define LORAG_DELAY    1000
#define LORA1_DELAY    1000
#define LORA2_DELAY    1000

#define ESPNOW1_ACT    
#define ESPNOW2_ACT    
#define ESPNOWG_ACT    
#define SERIAL_ACT     
#define MQTT_ACT       
#define LORAG_ACT   
#define LORA1_ACT 
#define LORA2_ACT     

//#define USE_LORA      
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//#define USE_LED
#define LED_PIN    32
#define NUM_LEDS    4

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE

#ifdef CREDENTIALS
#include <FDRScredentials.h>
#define WIFI_NET mySSID  // ssid of your accesspoint
#define WIFI_PASS myPASSWORD  // password of access point
#define MQTT_ADDR myMQTT_BROKER
#define BAND myBAND
#define SF mySF
#else
#define WIFI_NET "Your SSID"
#define WIFI_PASS "Password"
#define MQTT_ADDR "192.168.0.8"
#define BAND 915E6
#define SF 7
#endif
