//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration
//  (This file will soon be known as 'sensor_config.h')
//

#include <fdrs_globals.h> // Comment if you want to set specific values for this sensor in sensor_setup.h

#define READING_ID    1   //Unique ID for this sensor
#define GTWY_MAC      0x04 //Address of the nearest gateway

//#define USE_ESPNOW
#define USE_LORA
#define DEEP_SLEEP
//#define POWER_CTRL    14
#define DEBUG

//LoRa Configuration
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
//#define BAND 915E6
//#define SF 7
