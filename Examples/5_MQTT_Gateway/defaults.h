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

//#define RXD2 21
//#define TXD2 22

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

//#define USE_LED
#define LED_PIN    32
#define NUM_LEDS    4

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE
#define UART_IF Serial1
