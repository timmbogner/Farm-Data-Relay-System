# <p align="center">Farm Data Relay System
##### <p align="center">[***In loving memory of Gay Holman, an extraordinary woman.***](https://www.facebook.com/CFECI/posts/2967989419953119) #####

The Farm Data Relay System is an easy way to link remote sensors to the internet without the need for WiFi. It uses the ESP-NOW protocol on widely availabe ESP32 and ESP8266 microcontroller boards, and can be used to collect and transmit sensor data in situations where it would be too difficult or energy-consuming to provide full WiFi coverage. 

A major consideration has been to make the system straight-forward and easy to set up, since the primary user may not be very familiar with Arduino. Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code. 

Other than the nodes, each device in the system has a one-byte address. At boot, each device changes its MAC address to "AA:BB:CC:DD:EE:xx" where xx is the device's address identifier. Nodes can send their data to a terminal, a relay, or directly to a gateway. Each of these devices can be connected as needed in a chain that leads to a gateway, where the data is handed off to another system.

## Getting Started  
To get started using FDRS with Node-Red, you'll first need to flash an ESP32 or 8266 device with the FDRS_Gateway sketch. Next, connect the device via serial port to a computer running Node-Red. You can then use the serialport node connected to a JSON node, and finally a split node to get your data ready to use. You can find the basic flow examples in the 'NodeRed-Basic.json' file. If you are using a usb-serial port, keep in mind that you may have to change the selected port after plugging in your device, or de-select the port when trying to flash the device from another program.

As you flash each sensor, you'll need to change the UNIT_ID to a unique integer (0-65535). For some sensors, you'll also need to adjust pin assignments and install libraries.
By default, all sensors have GTWY_MAC set to 0x00. You'll only need to change this if you are going to use a relay to extend the system's range.

If you need to get data from sensors that are out of reach of the gateway, place an FDRS_Relay device near the edge of the gateway's range. Flash this device with the default parameters, and then flash the sensors with GTWY_MAC set to 0x01. They will now send their data to the relay, which in turn sends it to the gateway.

## Sensors
```
typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each sensor in the system sends its data over ESP-NOW as a float 'd' inside of a structure called a DataReading. Its global sensor address is represented by an integer 'id', and each type is represented by a single byte 't'.  If sensors need to send multiple types of readings (ex: temp and humidity), then they are sent in an array of DataReadings. In this case each reading will share an id, but be differentiated by its type.

## Terminal
A terminal is a device that recieves data from the nodes and aggregates it into a larger array. The larger array is then periodically sent forward through the system to the gateway. The time between sends must be short enough so as not to  exceed the maximum legnth of an ESP-NOW packet with DataReadings, which is 31.


## Relays
Relays are absolute ESP-NOW repeaters. They are programmed with the address of the previous and next device, and when data is received from one, it delivers it to the other. The "DEFT_MAC" setting defines where a relay will send an incoming packet with an unknown address. This can be used to direct sensor packets either to the terminal or the gateway.

In the FDRS, one can place as many relays as needed to span the distance to the gateway, which is the final device in the system.

## Gateway
The gateway takes the packet of DataReadings and interprets it into json format, then outputs it over the serial port. From here it can be collected by another microcontroller or a Raspberry Pi front-end to be sent to the cloud for further processing and storage.




