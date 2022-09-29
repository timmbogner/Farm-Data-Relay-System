//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration

#include <fdrs_globals.h>

#define READING_ID    1   //Unique ID for this sensor
#define GTWY_MAC      0x01 //Address of the nearest gateway

#define USE_ESPNOW
//#define USE_LORA
#define DEEP_SLEEP
//#define POWER_CTRL    14
#define FDRS_DEBUG
#define DEBUG_NODE_CONFIG // Displays full config info on startup

//SPI Configuration -- Needed only on chipsets with multiple SPI interfaces (ESP32)
#define SPI_SCK 5
#define SPI_MISO 19
#define SPI_MOSI 27

//LoRa Configuration
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
//#define LORA_BAND 915E6         // LoRa Frequency Band
//#define LORA_SF 7               // LoRa Spreading Factor
//#define LORA_TXPWR 17           // LoRa TX power in dBm (+2dBm - +20dBm), default is +17dBm
//#define LORA_ACK                // Request LoRa acknowledgment. Increases battery usage.
//#define LORA_ACK_TIMEOUT 400    // LoRa ACK timeout in ms. (Minimum = 200)
//#define LORA_RETRIES 2          // LoRa ACK retries [0 - 3]
