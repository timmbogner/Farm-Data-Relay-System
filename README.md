# Farm Data Relay System

The purpose of the Farm Data Relay System is to provide a method of exchanging data between many ESP32 or ESP8266 devices in situations where it would be difficult or energy-consuming to provide full WiFi coverage. 

A major goal is to make the system straight-forward and easy to use, as the primary user may not necessarily be very familiar with Arduino. Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code. 
Other than the sensors, each device in the system has a one-byte address. To make things easier, it's good to keep their addresses consecutive with their order in the system. At boot, each device changes its MAC address to "AA:BB:CC:DD:EE:xx" where xx is the device's address identifier.
There are three types of units in the FDRS: the terminal, the relays, and the gateway.

## Nodes
Each "node" in the system sends its data as a float 'd' inside of a structure called a DataReading. Its global address is represented by an integer 'id', and has a type represented by a single byte 't'. For example: a BME280 sketch using FDRS will send an array of three DataReadings, one each for temperature, humidity, and pressure. The types for those readings are 1, 2, and 3 respectively.
I'm experimenting with nodes that *recieve* DataReadings as well. DataReadings of type 201 are treated as commands.

## Terminal
The terminal is a device that recieves data from the nodes and aggregates them into a larger array. The larger array is then periodically sent through the system to the gateway. The time between sends must be short enough so as not to  exceed the maximum legnth of an ESP-NOW packet with DataReadings, which is 31.

As I implement the ability to send commands, the terminal has taken on some routing resposibilities as well. When a terminal recieves a DataReading of type 200, it associates the last byte of the incoming MAC with the ID of the reading in an array. This array is referenced in order for the terminal to forward any commands it recieves to their proper destinations.

## Relays
Relays are absolute ESP-NOW repeaters. They are programmed with the address of the previous and next device, and when data is received from one, it delivers it to the other. The "DEFT_MAC" setting defines where a relay will send an incoming packet with an unknown address. This can be used to direct sensor packets either to the terminal or the gateway.

In the FDRS, one can place as many relays as needed to span the distance to the gateway, which is the final device in the system. Being honest, I now have access to WiFi close to my terminal, so I no longer need any relays at all. They're still handy though.

## Gateway
The gateway takes the packet of DataReadings and interprets it into json format, then outputs it over the serial port. From here it can be collected by another microcontroller or a Raspberry Pi front-end to be sent to the cloud for further processing and storage.

## Front-end
### Node Red
I'm very excited to have begun learning about and supporting Node Red via Raspberry Pi. I currently have soldered the serial and power connections of a WeMos D1 Mini directly to my RPi Zero W. I am getting pretty busy with actual farming, so this will make it a lot easier to develop front-end applications in the short-term.
### Blynk
This is for the original Blynk! I won't have time to learn the new system for a while, so it will probably remain in this state for a bit...

Each DataReading that is recieved via serial is set to its blynk virtual pin counterpart. I've also implemented "set" and "get" commands for the blynk terminal, but these are still experimental.

The Blynk sketch has been giving me problems, but it does still work. It suffers from a couple seemingly random WDT resets that I'm still struggling to pin down. This doesn't affect much functionality much for me, but I'd graciously accept any advice in that area.

## Future plans
Future plans are to include as many types of nodes and front end modules as I can fathom. I'm working on sketches to remotely adjust irrigation/ventilation, as well as an idea for RGB lanterns, with colors controlled by FDRS. I'm also looking into the ability to display certain readings on a screen and send manual commands from the terminal itself, or other devices within the system locally.

## Thank you
...very much for checking out my project! There are a few topics I've glossed over here that I intend to elaborate on in the future. If you have any questions, please feel free to contact me at bogner1@gmail.com.

Also a big thank you to both ***Sola Gratia Farm*** and ***The IDEA Store*** of *Urbana, IL, USA*, without whom this project would not exist.


[***In loving memory of Gay Holman, an extraordinary individual.***](https://www.facebook.com/CFECI/posts/2967989419953119) 
