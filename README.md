# Farm-Data-Relay-System

I designed this system for the purpose of retrieving sensor data from my community farm, which is out of range of any WiFi access point.

Each sensor is connected to an ESP8266, which sends its data reading, along with a unique identifier to another ESP8266 known as the terminal via ESP-NOW. The terminal collects each reading in an array, and periodically submits them to a data relay. Each data relay is programmed with the address of a device both upsteam and downstream, and will simply send every ESP-NOW packet it recieves to the opposite of where it came. The final relay should be within distance of the gateway, which accesses the cloud and does something with the data.
