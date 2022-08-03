//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration

#include <fdrs_globals.h>

#define READING_ID    0x02   //Unique ID for this sensor - 8 bits hexidecimal
#define GTWY_MAC      0x03 //Address of the nearest gateway

#define USE_ESPNOW
//#define USE_LORA
#define DEEP_SLEEP
//#define POWER_CTRL    14
#define FDRS_DEBUG

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
//#define LORA_BAND 915E6     // LoRa Frequency Band
//#define LORA_SF 7           // LoRa Spreading Factor
//#define LORA_TXPWR 17       // LoRa TX power in dBm (+2dBm - +20dBm), default is +17dBm.  Lower power = less battery use
//#define LORA_ACK      // Uncomment to enable request for LoRa ACKs at cost of increased battery usage
//#define LORA_ACK_TIMEOUT 400   // ms timeout waiting for LoRa ACKs (if enabled).  Wouldn't go less than 200ms
//#define LORA_RETRIES 2          // [0 - 3] When ACK enabled, number of sensor node tx retries when ACK not received or invalid CRC 
