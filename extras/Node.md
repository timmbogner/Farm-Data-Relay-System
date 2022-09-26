# FDRS User Node
A node is a device that sends and receives data from a nearby gateway. A node can be a sensor, controller, or both.

**NOTE: Controller node functionality is currently restricted to the *ESP-NOW protocol*. LoRa gateways can still transport data bi-directionally, but you will need to use ESP-NOW to register a controller node with a gateway.**

# Commands
### ``` beginFDRS();```
Initializes FDRS, powers up the sensor array, and begins ESP-NOW and/or LoRa.
## Sensor Commands
### ```loadFDRS(float d, uint8_t t);```
Loads some data into the current packet. 'd' is a float and 't' is a byte used to represent the sensor type. Type definitions can be found below. Please feel free to contact me if you'd like to add a new sensor type.
### ```bool sendFDRS();```
Sends the current packet using ESP-NOW and/or LoRa. Returns true if packet is confirmed to have been recieved successfully by the gateway.
### ``` sleepFDRS(int sleep_time)```
If available and enabled, the device enters deep-sleep. If ```#DEEP_SLEEP``` is disabled, the device will use a delay instead. ```sleep_time``` is entered in seconds.
## Controller Commands
### ```addFDRS(int timeout, void callback);```
Adds the device to the gateway's peer registry. This enables the device to receive transmissions from the gateway. ```timeout``` is in milliseconds. ```callback``` should be the name of the function that will recieve all incoming transmissions. The ESP-NOW Controller example demonstrates functionality.
### ```subscribeFDRS(uint16_t sub_id)``` 
Sets the device to listen for a specific DataReading id. When a DataReading with id ```sub_id``` is received, the callback function will be called and given the full DataReading as a parameter.
### ```unsubscribeFDRS(uint16_t sub_id)``` 
Removes ```sub_id``` from subscription list.

## Basic usage:
### Sensor
Sensor nodes load a packet with data, then send the packet to the gateway that they are addressed to.
```
void setup() {
  beginFDRS();
}
void loop() {
  loadFDRS(21.0, TEMP_T);
  sendFDRS();
  sleepFDRS(10);  //Sleep time in seconds
}
```

### Controller
Controller nodes register with the gateway they are addressed to, then receive data from it. 

```
void fdrs_recv_cb(DataReading theData) {
  //Quickly handle incoming data
}

void setup() {
  beginFDRS();
  //pingFDRS(1000);
  addFDRS(1000, fdrs_recv_cb);
  subscribeFDRS(READING_ID);
}
void loop() {
}
```

## Configuration

### ```#define READING_ID  n```
The identifier of this individual device. Should be a 16 bit integer value (0 - 65535). Controllers are not necessarily tied to this parameter, and can be subscribed to up to 256 different IDs. Sensors will likely be treated similarly in the future, allowing the user to send sensor readings under multiple IDs.
### ```#define GTWY_MAC  0xnn```
The UNIT_MAC of the gateway that this device will communicate with.
### ```#define FDRS_DEBUG```
This definition enables debug messages to be sent over the serial port. If disabled, no serial debug interface will be initialized. 
### ```#define USE_ESPNOW```
Enables/disables ESP-NOW.
### ```#define USE_LORA```
Enables/disables LoRa.
### ```#define DEEP_SLEEP```
If enabled, device will enter deep-sleep when the sleepFDRS() command is used. If using ESP8266, be sure that you connect the WAKE pin (GPIO 16) to RST or your device will not wake up. 
### ```#define POWER_CTRL (pin)```
If defined, power control will bring a GPIO pin high when FDRS is initialized. This is useful for powering sensors while running on battery.

## Callback function
The callback function is executed when data arrives with an ID that the controller is subscribed to, interrupting all other tasks. Inside of this function, the user has access to the incoming DataReading.

This function should **ONLY** contain the code needed to save the data to a more permanent location. *Any interpretation or display of the data should occur outside of the callback function*.

Intermediate users may also like to know that if the controller is subscribed to multiple IDs, the callback may be called multiple times before returning to the loop().

## Type Definitions 
For the moment, my thought is to reserve the first two bits of the type. I might use them in the future to indicate the data size or type (bool, char,  int, float, etc?). This leaves us with 64 possible type definitions. If you have more types to add, please get in touch!
```
#define STATUS_T        0  // Status 
#define TEMP_T          1  // Temperature 
#define TEMP2_T         2  // Temperature #2
#define HUMIDITY_T      3  // Relative Humidity 
#define PRESSURE_T      4  // Atmospheric Pressure 
#define LIGHT_T         5  // Light (lux) 
#define SOIL_T          6  // Soil Moisture 
#define SOIL2_T         7  // Soil Moisture #2 
#define SOILR_T         8 // Soil Resistance 
#define SOILR2_T        9 // Soil Resistance #2 
#define OXYGEN_T        10 // Oxygen 
#define CO2_T           11 // Carbon Dioxide
#define WINDSPD_T       12 // Wind Speed
#define WINDHDG_T       13 // Wind Direction
#define RAINFALL_T      14 // Rainfall
#define MOTION_T        15 // Motion
#define VOLTAGE_T       16 // Voltage
#define VOLTAGE2_T      17 // Voltage #2
#define CURRENT_T       18 // Current
#define CURRENT2_T      19 // Current #2
#define IT_T            20 // Iterations
#define LATITUDE_T      21 // GPS Latitude
#define LONGITUDE_T     22 // GPS Longitude
#define ALTITUDE_T      23 // GPS Altitude
#define HDOP_T          24 // GPS HDOP
#define LEVEL_T         25 // Fluid Level
```
## Under the hood
```
typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each node in the system sends its data inside of a structure called a DataReading. Its global sensor address is represented by an integer 'id', and each type of reading is represented by a single byte 't'.  If a sensor or gateway needs to send multiple DataReadings, then they are sent in an array. A single DataReading.id may have readings of multiple types ('t') associated with it.
