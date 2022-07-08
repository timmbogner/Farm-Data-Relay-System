//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration
//  (This file will soon be known as 'sensor_config.h')
//

#include <fdrs_globals.h> //Uncomment when you install the globals file

#define READING_ID    1   //Unique ID for this sensor
#define GTWY_MAC      0x04 //Address of the nearest gateway

//#define USE_ESPNOW
#define USE_LORA
#define DEEP_SLEEP
//#define POWER_CTRL    14
#define FDRS_DEBUG

//SPI Configuration -- Needed only on Boards with multiple SPI interfaces like the ESP32
#define SPI_SCK 5
#define SPI_MISO 19
#define SPI_MOSI 27

//LoRa Configuration
#define LORA_SS 18
// RST: 23 for T-Beam or Paxcounter, 14 for Lora32 V1
#define LORA_RST 23
#define LORA_DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define LORA_BAND 866E6
#define LORA_SF 7
