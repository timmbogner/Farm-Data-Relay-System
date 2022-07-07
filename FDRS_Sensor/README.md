# Sensor 2.000
This file is an example of how to set up a sensor using FDRS. 


## Commands
### ``` beginFDRS();```
Initializes FDRS, powers up the sensor array, and begins ESP-NOW and/or LoRa.
### ```loadFDRS(float d, uint8_t t);```
Loads some data into the current packet. 'd' is a floating-point integer and 't' is a byte used to represent the sensor type. Type definitions can be found below. Please feel free to contact me if you'd like to add a new sensor type.
### ```sendFDRS();```
Sends the current packet using ESP-NOW and/or LoRa.
### ``` sleepFDRS(int sleep_time)```
If available and enabled, the device enters deep-sleep. If ```#DEEP_SLEEP``` is disabled, the device will use a delay instead. ```int sleep_time``` is entered in seconds.

## Options

### ```#define READING_ID  n```
The identifier of this individual device. Should be a 16-bit integer (0-65535). 
### ```#define GTWY_MAC  0xnn```
The UNIT_MAC of the gateway that this device will send its data to.
### ```#define FDRS_DEBUG```
This definition enables debug messages to be sent over the serial port. If disabled, no serial interface will be initialized. 
### ```#define USE_ESPNOW```
Enables/disables ESP-NOW.
### ```#define USE_LORA```
Enables/disables LoRa.
### ```#define DEEP_SLEEP```
If enabled, device will enter deep-sleep when the sleepFDRS() command is used. If using ESP8266, be sure that you connect the WAKE pin (GPIO 16) to RST or your device will not wake up. 
### ```#define POWER_CTRL (pin)```
IF defined, power control will bring a GPIO pin high within beginFDRS(). This is useful for powering sensors while running on battery.
## Type Definitions 
For the moment, my thought is to reserve the first two bits of the type. I might use them in the future to indicate the data size or type (bool, char,  int, float, etc?). This leaves us with 64 possible type definitions. If you have more types to add, please get in touch!
```
#define STATUS_T        0  // Status 
#define TEMP_T          1  // Temperature 
#define TEMP2_T         2  // Temperature #2
#define HUMIDITY_T      3  // Relative Humidity 
#define PRESSURE_T      4  // Atmospheric Pressure 
#define LIGHT_T         5  // Light (lux) 
#define SOIL_T          6  // Soil Moisture 
#define SOIL2_T         7  // Soil Moisture #2 
#define SOILR_T         8 // Soil Resistance 
#define SOILR2_T        9 // Soil Resistance #2 
#define OXYGEN_T        10 // Oxygen 
#define CO2_T           11 // Carbon Dioxide
#define WINDSPD_T       12 // Wind Speed
#define WINDHDG_T       13 // Wind Direction
#define RAINFALL_T      14 // Rainfall
#define MOTION_T        15 // Motion
#define VOLTAGE_T       16 // Voltage
#define VOLTAGE2_T      17 // Voltage #2
#define CURRENT_T       18 // Current
#define CURRENT2_T      19 // Current #2
#define IT_T            20 // Iterations
#define LATITUDE_T      21 // GPS Latitude
#define LONGITUDE_T     22 // GPS Longitude
#define ALTITUDE_T      23 // GPS Altitude

```
## Under the hood
```
typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each sensor in the system sends its data over ESP-NOW or LoRa as a float 'd' inside of a structure called a DataReading. Its global sensor address is represented by an integer 'id', and each type of reading is represented by a single byte 't'.  If a sensor or gateway needs to send multiple DataReadings, then they are sent in an array. A single DataReading.id may have readings of multiple types ('t') associated with it.
