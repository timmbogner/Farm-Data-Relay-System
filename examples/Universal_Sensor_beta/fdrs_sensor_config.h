//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration File
//	By default global settings will be used for this sensor.
//	If you need to change a specific setting, uncomment and set to your needs further below.
//

#define FDRS_DEBUG 			// Comment, if you do not want to see debug messages
#include <fdrs_globals.h> 	// Comment if you want to set specific values for this sensor here

#define READING_ID    1   	//Unique ID for this sensor
#define GTWY_MAC      0x04 	//Address of the nearest gateway

#define DEEP_SLEEP
//#define POWER_CTRL    14

// TODO: New, taken from Gateway - should be configured in the same way as there!
//Pins for UART data interface (ESP32 only)
#define RXD2 14
#define TXD2 15

// SPI Configuration -- Needed only on Boards with multiple SPI interfaces like the ESP32
#define SPI_SCK 5
#define SPI_MISO 19
#define SPI_MOSI 27

// LoRa Pin Configuration -- Uncomment and configure if you are using LoRa
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26


// LoRa Transport Configuration -- This should be set in the global configuration file. However, if you need a
// specific band and spreading factor for this sensor, configure below.
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
//#define LORA_BAND 915E6
//#define LORA_SF 7

// TODO: Addd local MQTT configuration