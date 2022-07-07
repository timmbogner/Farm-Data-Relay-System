//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration
//  (This file will soon be known as 'sensor_config.h')
//

//#include <fdrs_globals.h> //Uncomment when you install the globals file

#define READING_ID    1   //Unique ID for this sensor
#define GTWY_MAC      0x04 //Address of the nearest gateway

#define DEEP_SLEEP
//#define POWER_CTRL    14
#define DEBUG

//LoRa Configuration
#define SPI_SCK 5
#define SPI_MISO 19
#define SPI_MOSI 27
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define LORA_BAND 915E6
#define LORA_SF 7
