//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration

#include <fdrs_globals.h>

#define READING_ID    1   //Unique ID for this sensor
#define GTWY_MAC      0x04 //Address of the nearest gateway

//#define USE_ESPNOW
#define USE_LORA
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
#define LORA_BAND 915E6
#define LORA_SF 7
#define LORA_ACK      // Uncomment to enable request for LoRa ACKs at cost of increased battery usage
#define LORA_NOACK_CRC 0xFFFF  // CRC value to be used when ACKs are disabled to tell gateway we do not want ACK
#define LORA_ACK_TIMEOUT 400   // ms timeout waiting for LoRa ACKs (if enabled).  Wouldn't go less than 200ms
#define LORA_RETRIES 2          // [0 - 3] When ACK enabled, number of sensor node tx retries when ACK not received or invalid CRC 
