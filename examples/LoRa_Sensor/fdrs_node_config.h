//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration

#include <fdrs_globals.h>

#define READING_ID    2   //Unique ID for this sensor
#define GTWY_MAC      0x01 //Address of the nearest gateway

//#define USE_ESPNOW
#define USE_LORA
#define DEEP_SLEEP
//#define POWER_CTRL    14
#define FDRS_DEBUG

// LoRa Configuration
#define RADIOLIB_MODULE SX1276
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DIO   26
#define LORA_BUSY  33
//#define USE_SX126X

#define LORA_TXPWR 17   // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))
#define LORA_ACK        // Request LoRa acknowledgment.

//#define CUSTOM_SPI
#define LORA_SPI_SCK 5
#define LORA_SPI_MISO 19
#define LORA_SPI_MOSI 27