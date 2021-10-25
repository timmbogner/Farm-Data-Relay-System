# Farm Data Relay System

The Farm Data Relay System is an easy way to link remote sensors to the internet without the need for WiFi. It uses the ESP-NOW protocol on widely availabe ESP32 and ESP8266 microcontroller boards, and can be used to collect and transmit sensor data in situations where it would be too difficult or energy-consuming to provide full WiFi coverage. 

A major consideration has been to make the system straight-forward and easy to set up, since the primary user may not be very familiar with Arduino. Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code. 

Other than the nodes, each device in the system has a one-byte address. At boot, each device changes its MAC address to "AA:BB:CC:DD:EE:xx" where xx is the device's address identifier. Nodes can send their data to a terminal, a relay, or directly to a gateway. Each of these devices can be connected as needed in a chain that leads to a gateway, where the data is handed off to another system.

## Nodes
```
typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each node in the system sends its data over ESP-NOW as a float 'd' inside of a structure called a DataReading. Its global address is represented by an integer 'id', and has a type represented by a single byte 't'.  If a sensor is capable of multiple types of readings (ex: temp and humidity), they are send in an array.

## Terminal
A terminal is a device that recieves data from the nodes and aggregates it into a larger array. The larger array is then periodically sent forward through the system to the gateway. The time between sends must be short enough so as not to  exceed the maximum legnth of an ESP-NOW packet with DataReadings, which is 31.


## Relays
Relays are absolute ESP-NOW repeaters. They are programmed with the address of the previous and next device, and when data is received from one, it delivers it to the other. The "DEFT_MAC" setting defines where a relay will send an incoming packet with an unknown address. This can be used to direct sensor packets either to the terminal or the gateway.

In the FDRS, one can place as many relays as needed to span the distance to the gateway, which is the final device in the system.

## Gateway
The gateway takes the packet of DataReadings and interprets it into json format, then outputs it over the serial port. From here it can be collected by another microcontroller or a Raspberry Pi front-end to be sent to the cloud for further processing and storage.

## Front-end
### Node-Red
I have really been enjoying the flexibility of Node-red for collecting and displaying the data. My current setup is a work in progress, but I'd be interested in feedback.
### Blynk
This is for the original Blynk! I won't have time to learn the new Blynk system for a while, so it will probably remain in this state for a bit...

## Future plans
Future plans are to include as many types of nodes and front end modules as I can fathom. I'm also working on sketches to remotely adjust irrigation/ventilation, as well as an idea for RGB lanterns, with colors controlled by FDRS.

## Thank you
...very much for checking out my project! There are a few topics I've glossed over here that I intend to elaborate on in the future. If you have any questions, please feel free to contact me at bogner1@gmail.com.

A big thank you to both ***Sola Gratia Farm*** and ***The IDEA Store*** of *Urbana, IL, USA*, without whom this project would not exist.


[***In loving memory of Gay Holman, an extraordinary individual.***](https://www.facebook.com/CFECI/posts/2967989419953119) 
