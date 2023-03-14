# FDRS Node
A node is a device that sends and receives data from a nearby gateway. A node can be a **sensor**, **controller**, ***or both***.
## Addresses
#### ```#define READING_ID  n```
The unique ID that the node will use when sending sensor values. Can be any integer 0 - 65535. Nodes are not necessarily tied to this parameter. They can be subscribed to up to 256 different IDs or send using several different IDs.
#### ```#define GTWY_MAC  0xnn```
The ```UNIT_MAC``` of the gateway that this device will be paired with.

## Usage
#### ```beginFDRS();```
Initializes FDRS, powers up the sensor array, and begins ESP-NOW and/or LoRa.
#### ```uint32_t pingFDRS(timeout);```
Sends a ping request to the device's paired gateway with a timeout in ms. Returns the ping time in ms as well as displaying it on the debugging console.

## Sensor API
#### ```loadFDRS(float data, uint8_t type, uint16_t id);```
Loads some data into the current packet. 'data' is a float, 'type' is the data type (see below), and 'id' is the DataReading id.
#### ```loadFDRS((float data, uint8_t type);```
Same as above, but the 'id' is preset to the node's ```READING_ID```. 
#### ```bool sendFDRS();```
Sends the current packet using ESP-NOW and/or LoRa. Returns true if packet is confirmed to have been recieved successfully by the gateway.
#### ```sleepFDRS(seconds)```
Time to sleep in seconds. If ```DEEP_SLEEP``` is enabled, the device will enter sleep. Otherwise it will use a simple ```delay()```.

## Controller API
#### ```addFDRS(void callback);```
Initializes controller functionality by selecting the function to be called when incoming commands are recieved. If using LoRa, the controller will automatically recieve any packets sent with broadcastLoRa(), provided they were sent by the paired gateway. ESP-NOW requires the device to register with its gateway before it will recieve incoming commands. This is done automatically, and the ESP-NOW node will continue recieving data until the paired gateway is reset. A maximum of 16 ESP-NOW controllers can recieve data from a single gateway. There is no limit to how many LoRa controllers can listen to the same gateway.
#### ```subscribeFDRS(uint16_t sub_id);``` 
Sets the device to listen for a specific DataReading id. When a DataReading with id ```sub_id``` is received, the callback function will be called and given the full DataReading as a parameter.
#### ```unsubscribeFDRS(uint16_t sub_id);``` 
Removes ```sub_id``` from subscription list.
#### ```loopFDRS();``` 
Always add this to ```loop()``` to handle the controller's listening capabilities.

## Basic Examples:
### Sensor
Sensors load a packet with data, then send the packet to the gateway that they are addressed to.
``` cpp
#include "fdrs_node_config.h"
#include <fdrs_node.h>

void setup() {
  beginFDRS();  // Start the system
  pingFDRS(2000); // Send ping and wait 2000ms for response
}
void loop() {
  loadFDRS(21.0, TEMP_T); // Load a temperature of 21.0 into the queued packet
  sendFDRS();        // Send the queued packet
  sleepFDRS(60); // Sleep for 60 seconds
}
```

### Controller
Controllers register with the gateway they are addressed to, then receive data from it. 

``` cpp
#include "fdrs_node_config.h"
#include <fdrs_node.h>

void fdrs_recv_cb(DataReading theData) {
  DBG("ID: " + String(theData.id));
  DBG("Type: " + String(theData.t));
  DBG("Data: " + String(theData.d));
}

void setup() {
  beginFDRS();       // Start the system
  addFDRS(fdrs_recv_cb);  // Call fdrs_recv_cb() when data arrives.
  subscribeFDRS(READING_ID);  // Subscribe to DataReadings with ID matching READING_ID
}
void loop() {
  loopFDRS();  // Listen for data
}
```

## Configuration

#### ```#define USE_ESPNOW```
Enables/disables ESP-NOW.
#### ```#define USE_LORA```
Enables/disables LoRa.
#### ```#define LORA_ACK```
Enables LoRa packet acknowledgement. The device will use CRC to ensure that the data arrived at its destination correctly. If disabled, ```sendFDRS()``` will always return true when sending LoRa packets.

Thanks to [aviateur17](https://github.com/aviateur17) for this feature!
#### ```#define FDRS_DEBUG```
This definition enables debug messages to be sent over the serial port. If disabled, no serial debug interface will be initialized. 
#### ```#define DEEP_SLEEP```
If enabled, device will enter deep-sleep when the sleepFDRS() command is used. If using ESP8266, be sure that you connect the WAKE pin (GPIO 16) to RST or your device will not wake up. 
#### ```#define POWER_CTRL n```
If defined, power control will bring a GPIO pin high when FDRS is initialized. This is useful for powering sensors while running on battery.
#
## LoRa Configuration
#### ```#define RADIOLIB_MODULE cccc```
The name of the RadioLib module being used. Tested modules: SX1276, SX1278, SX1262.
#### ```#define LORA_SS n```
LoRa chip select pin.
#### ```#define LORA_RST n```
LoRa reset pin.
#### ```#define LORA_DIO n```
LoRa DIO pin. This refers to DIO1 on SX127x chips and DIO1 on SX126x chips.
#### ```#define LORA_BUSY n```
For SX126x chips: LoRa BUSY pin. For SX127x: DIO1 pin, or "RADIOLIB_NC" to leave it blank. 
#### ```#define LORA_TXPWR n```
LoRa TX power in dBm.
#### ```#define USE_SX126X```
Enable this if using the SX126x series of LoRa chips.
#
### SSD1306 OLED Display
Built on the [ThingPulse OLED SSD1306 Library](https://github.com/ThingPulse/esp8266-oled-ssd1306)
##### ```#define OLED_HEADER "cccc"```
The message to be displayed at the top of the screen.
#### ```#define OLED_SDA n``` and ```OLED_SCL n```
OLED I²C pins.
#### ```#define OLED_RST n```
OLED reset pin. Use '-1' if not present or known.
#
## Callback Function
The callback function is executed when data arrives with an ID that the controller is subscribed to. Inside of this function, the user has access to the incoming DataReading. If multiple readings are recieved, the function will be called for each of them. While you should always be brief in interrupt callbacks (ISRs), it's okay to do more in this one.
## Type Definitions 
For the moment, my thought is to reserve the first two bits of the type. I might use them in the future to indicate the data size or type (bool, char,  int, float, etc?). This leaves us with 64 possible type definitions. If you have more types to add, please get in touch!
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
#define HDOP_T          24 // GPS HDOP
#define LEVEL_T         25 // Fluid Level
#define UV_T            26 // UV
#define PM1_T           27 // 1 Particles
#define PM2_5_T         28 // 2.5 Particles
#define PM10_T          29 // 10 Particles
#define POWER_T         30 // Power
#define POWER2_T        31 // Power #2
#define ENERGY_T        32 // Energy
#define ENERGY2_T       33 // Energy #2
```
## Under the Hood
```
typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each node in the system sends its data inside of a structure called a DataReading. Its global sensor address is represented by an integer 'id', and each type of reading is represented by a single byte 't'.  If a sensor or gateway needs to send multiple DataReadings, then they are sent in an array. A single DataReading.id may have readings of multiple types ('t') associated with it.
