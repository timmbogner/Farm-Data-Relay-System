//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration


#define READING_ID    1   //Unique ID for this sensor
#define GTWY_MAC      0x03 //Address of the nearest gateway

//#define USE_ESPNOW
#define USE_LORA
//#define DEEP_SLEEP
//#define POWER_CTRL    14

// Choose none or one of the debug options below.  
// DEBUG will show some information but for troubleshooting a good level would be FDRS_DEBUG_1
#define FDRS_DEBUG     // Enable USB-Serial debugging
// #define FDRS_DEBUG_1 // Additional logging messages for troubleshooting purposes - commment out the other FDRS_DEBUG options
// #define FDRS_DEBUG_2 // Show all logging messages for development purposes - comment out the other FDRS_DEBUG options

// LoRa Configuration
#define RADIOLIB_MODULE SX1276  // ESP32 SX1276 (TTGO)
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DIO   26
#define LORA_BUSY  33

#define LORA_TXPWR 17    // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))
#define LORA_ACK        // Request LoRa acknowledgment.

#define USE_OLED    
#define OLED_HEADER "FDRS"
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16