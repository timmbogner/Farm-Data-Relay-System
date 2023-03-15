//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration


#define READING_ID  21   //Unique ID for this sensor
#define GTWY_MAC    0x01 //Address of the nearest gateway

//#define USE_ESPNOW
#define USE_LORA

//#define USE_LR  // Enables  802.11LR on ESP32 ESP-NOW devices

//#define DEEP_SLEEP
//#define POWER_CTRL  14
#define FDRS_DEBUG

// LoRa Configuration
#define RADIOLIB_MODULE SX1276  // ESP32 SX1276 (TTGO)
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DIO   26
#define LORA_BUSY  33
//#define USE_SX126X

//#define CUSTOM_SPI
#define LORA_SPI_SCK  5
#define LORA_SPI_MISO 19
#define LORA_SPI_MOSI 27

#define LORA_TXPWR 17    // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))
#define LORA_ACK        // Request LoRa acknowledgment.

//#define USE_OLED    
#define OLED_HEADER "FDRS"
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16