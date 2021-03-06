# Farm Data Relay System

The goal of the Farm Data Relay System is to provide a way for sensor data to be collected from areas out of wifi range, while not straying from the popular and presentable ESP8266 or soon, the ESP32. I have tested ESP-NOW range to be reliable within at least 500 feet with line of sight, effectively double what would be possible with regular WiFi connections. Another goal is to make the system straight-forward and easy to use, as the primary user will not necessarily be familiar with Arduino.
Other than the sensors, each device in the system has a one-byte address. To make things easier, it's good to keep their addresses consecutive with their order in the system. At boot, each device changes its MAC address to "AA:BB:CC:DD:EE:xx" where xx is the device's address identifier.
There are three types of units in the FDRS: the terminal, the relays, and the gateway.

# Terminal
The terminal receives individual ESP-NOW packets from the sensor modules and stores them in an array. At the moment, this is arbitrarily set to a six-item array. At a set interval, the terminal sends its data array to the preprogrammed next device in the system, usually a relay.

# Relays
Relays are absolute ESP-NOW repeaters. They are programmed with the address of the previous and next device, and when data is received from one, it delivers it to the other. It also works in reverse, but for the moment the system only supports one-way communication.
In the FDRS, one can place as many relays as needed to span the distance to the final device in the system: the gateway.

# Gateway
The gateway takes the packet of ESP-NOW data and interprets it into JSON format, then outputs it over the serial port. From here it can be collected by another microcontroller or a Raspberry Pi for further processing.
I've included an ESP8266 sketch for receiving the JSON over serial and posting it to Blynk as well.

# Sensors
The sensors are pre-set with the address of the terminal and their own one-byte identifier. This ID is sent along with each data reading and will define its position in the larger data array at the terminal and beyond.
The code is currently all written for ESP8266. The NEXT major addition will be ESP32 support. The two work with ESP-NOW just slightly differently in Arduino, but I will be porting it very soon.
Future plans include two-way communication, some basic routing ability for the relays, and optimizing the underlying protocols needed for the first two goals. I'd like to eventually be able to send commands to remotely adjust irrigation/ventilation or even send weather data such as tornado warnings to the terminal in future iterations of this project.

# Thank you
...very much for checking out my project! There are a few topics I've glossed over here that I intend to elaborate on in the future. If you have any questions, please feel free to contact me at bogner1@gmail.com.

Also a big thank you to both Sola Gratia Farm and the IDEA Store of Urbana, IL, USA, without whom this project would not exist.

