#ifdef DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#define ESPNOW1_DELAY  0
#define ESPNOW2_DELAY  0
#define ESPNOWG_DELAY  0
#define SERIAL_DELAY   0
#define MQTT_DELAY     0
#define LORAG_DELAY    1000
#define LORA1_DELAY    1000
#define LORA2_DELAY    1000
  

//#define USE_LORA      
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
uint8_t LoRaAddress[] = {0x42, 0x00};  //Do not touch!!!

//#define USE_LED
#define LED_PIN    32
#define NUM_LEDS    4

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // do not touch!

#ifdef CREDENTIALS
#include <FDRScredentials.h>
#define WIFI_NET my_SSID  // ssid of your accesspoint
#define WIFI_PASS my_PASSWORD  // password of access point
#define MQTT_ADDR my_MQTT_BROKER
#define BAND my_BAND
#define SF my_SF
#else
#define WIFI_NET "Your SSID"
#define WIFI_PASS "Password"
#define MQTT_ADDR "192.168.0.8"
#define BAND 915E6
#define SF 7
#endif

#define STATUS_T    0  // Status 
#define TEMP_T      1  // Temperature 
#define TEMP2_T     2  // Temperature #2
#define HUMIDITY_T  3  // Relative Humidity 
#define PRESSURE_T  4  // Atmospheric Pressure 
#define LIGHT_T     5  // Light (lux) 
#define SOIL_T      6  // Soil Moisture 
#define SOIL2_T     7  // Soil Moisture #2 
#define SOILR_T      8 // Soil Resistance 
#define SOILR2_T     9 // Soil Resistance #2 
#define OXYGEN_T    10 // Oxygen 
#define CO2_T       11 // Carbon Dioxide
#define WINDSPD_T   12 // Wind Speed
#define WINDHDG_T   13 // Wind Direction
#define RAINFALL_T  14 // Rainfall
#define MOTION_T    15 // Motion
#define VOLTAGE_T   16 // Voltage
#define VOLTAGE2_T  17 // Voltage #2
#define CURRENT_T   18 // Current
#define CURRENT2_T  19 // Current #2
#define IT_T        20 // Iterations


#define TOPIC_DATA "FDRS/DATA"
#define TOPIC_STATUS "FDRS/STATUS"
#define TOPIC_COMMAND "FDRS/COMMAND" 


