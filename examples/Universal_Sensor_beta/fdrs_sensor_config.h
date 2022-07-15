//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration File
//	By default global settings defined in fdrs_globals.h will be used for all sensors.
//	If you need to change a specific setting, uncomment and set to your needs further below.
//
#ifndef  __FDRS_SENSOR_CONFIG__H__
#define __FDRS_SENSOR_CONFIG__H__

#define FDRS_DEBUG 			// Comment, if you do not want to see debug messages
#include <fdrs_globals.h> 	// Comment if you want to set specific values for this sensor here

#define READING_ID    1   	//Unique ID for this sensor
#define GTWY_MAC      0x04 	//Address of the nearest gateway

#define DEEP_SLEEP
//#define POWER_CTRL    14

// Uncomment the sensor type you want to use 
#define USE_LORA
//#define USE_ESPNOW

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


// LoRa Transport Configuration -- This should be globally configured in fdrs_globals.h. If you need a
// specific band and spreading factor for this sensor, configure below.
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
//#define LORA_BAND 915E6
//#define LORA_SF 7

// MQTT Configuration -- This should be globally configured in fdrs_globals.h. If you need to specify 
// a different MQTT server for this sensor, configure below.
//#define MQTT_ADDR "192.168.0.8"
//#define MQTT_AUTH   //uncomment to enable MQTT authentication 
//#define MQTT_USER   "Your MQTT Username"
//#define MQTT_PASS   "Your MQTT Password"

#endif //__FDRS_SENSOR_CONFIG__H__