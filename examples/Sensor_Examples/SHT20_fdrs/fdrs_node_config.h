//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration


#define READING_ID    11   //Unique ID for this sensor
#define GTWY_MAC      0x01 //Address of the nearest gateway

#define USE_ESPNOW
//#define USE_LORA
#define DEEP_SLEEP
//#define POWER_CTRL    14
#define FDRS_DEBUG

// LoRa Configuration
#define RADIOLIB_MODULE SX1276 //Tested on SX1276
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO 26

#define LORA_TXPWR 17    // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))
#define LORA_ACK        // Request LoRa acknowledgment.
