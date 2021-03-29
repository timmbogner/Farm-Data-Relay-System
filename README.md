# Farm Data Relay System

The goal of the Farm Data Relay System is to provide a way for sensor data to be collected from areas out of wifi range, while using only the popular and readily available ESP series of microcontrollers. It uses the ESP-NOW communication protocol to transmit and receive data over ranges that far-exceed those that would be possible using WiFi. The whole system could be described as a daisy chain of ESP devices.
Another goal is to make the system straight-forward and easy to use, as the primary user may not necessarily be familiar with Arduino. Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code.
Other than the sensors, each device in the system has a one-byte address. To make things easier, it's good to keep their addresses consecutive with their order in the system. At boot, each device changes its MAC address to "AA:BB:CC:DD:EE:xx" where xx is the device's address identifier.
There are three types of units in the FDRS: the terminal, the relays, and the gateway.

## Sensors
Data starts its life at a sensor, where it is stored as a float within an object caled a DataReading. This object also contains one-byte sensor type, and a two-byte global identifier. Multiple data readings are assembled into a DataPacket object, and sent forward via ESP-NOW to a pre-programmed terminal. *Although I use confusing terminology at the moment, sensors can also be directed to a relay or directly to the gateway.*

## Terminal
The terminal receives DataPackets from the sensors and compiles them into its own Datapacket.  At a set interval, the terminal sends its packet to the preprogrammed next device in the system.

## Relays
Relays are absolute ESP-NOW repeaters. They are programmed with the address of the previous and next device, and when data is received from one, it delivers it to the other. While I haven't added any functionality to send data backwards through the system, the "DEFT_MAC" setting defines where a relay will send an incoming packet with an unknown address. This can be used to direct sensor packets either to the terminal or the gateway.
In the FDRS, one can place as many relays as needed to span the distance to the final device in the system: the gateway.

## Gateway
The gateway takes the packet of ESP-NOW data and interprets it into Json format, then outputs it over the serial port. From here it can be collected by another microcontroller or a Raspberry Pi known as a front-end to be sent to the cloud for further processing and storage.

## Front-end
### Blynk
So far, the only front end module available is an example I wrote for sending the data to Blynk. I'll be adding more as I implement the system further.

## Future plans
Future plans include two-way communication and as many data sources as I can fathom. I'd like to eventually be able to send commands to remotely adjust irrigation/ventilation or even send weather data such as tornado warnings to the terminal in future iterations of this project.

Also, the purpose of the DataPacket wrapper will soon change, as there is no need to know the legnth of the array it contains anymore. I just wanted to get that out before someone mentioned it :o) 

## Thank you
...very much for checking out my project! There are a few topics I've glossed over here that I intend to elaborate on in the future. If you have any questions, please feel free to contact me at bogner1@gmail.com.

Also a big thank you to both ***Sola Gratia Farm*** and ***The IDEA Store*** of *Urbana, IL, USA*, without whom this project would not exist.

