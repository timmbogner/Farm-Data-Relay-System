#define GLOBAL_SSID "Your SSID"
#define GLOBAL_PASS "Password"
#define GLOBAL_MQTT_ADDR "192.168.0.8"

#define GLOBAL_BAND 915E6 //LoRa Frequency Band
#define GLOBAL_SF 7  //LoRa Spreading Factor

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
#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.
uint8_t LoRaAddress[] = {0x42, 0x00};  // do not change!!!

#define GLOBALS