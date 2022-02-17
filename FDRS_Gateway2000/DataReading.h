
typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

#define STATUS_T    0  // Status 
#define TEMP_T      1  // Temperature 
#define TEMP2_T     2  // Temperature #2
#define HUMIDITY_T  3  // Relative Humidity 
#define PRESSURE_T  4  // Atmospheric Pressure 
#define LIGHT_T     5  // Light (lux) 
#define SOIL_T      6  // Soil Moisture 
#define SOIL2_T     7  // Soil Moisture #2 
#define OXYGEN_T    8  // Oxygen 
#define CO2_T       9  // Carbon Dioxide
#define WINDSPD_T   10 // Wind Speed
#define WINDHDG_T   11 // Wind Direction
#define RAINFALL_T  12 // Rainfall
#define MOTION_T    13 // Motion
#define VOLTAGE_T   14 // Voltage
#define VOLTAGE2_T  15 // Voltage #2
#define CURRENT_T   16 // Current
#define CURRENT2_T  17 // Current #2
#define IT_T        18 // Iterations
