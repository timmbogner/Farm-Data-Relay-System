# Farm-Data-Relay-System

I designed this system for the purpose of retrieving sensor data from my community farm, which is out of range of any WiFi access point.

Each sensor is connected to an ESP8266, which sends its data reading, along with a unique identifier to another ESP8266 known as the terminal via ESP-NOW. The terminal collects each reading in an array, and periodically submits the array to a data relay. Each data relay is programmed with the address of a device both upsteam and downstream, and will simply send every ESP-NOW packet it recieves to the opposite of where it came. The final relay should be within distance of the gateway, which accesses the cloud and does something with the data. In this case, the gateway sets some variables within Blynk.

If you decide to try this code, please drop me a line at bogner1@gmail.com to let me know how it's going, or ask any questions. I plan to continue expanding it throughout this winter, including adding the ability to send messages back through the relays to control devices.

