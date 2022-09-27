# <p align="center">Farm Data Relay System
##### <p align="center">[***In loving memory of Gay Holman, an extraordinary woman.***](https://www.facebook.com/CFECI/posts/2967989419953119) #####

The Farm Data Relay System is an easy way to collect data from remote sensors without relying on WiFi. It is based around the ESP-NOW protocol, which is readily available on ESP32 and ESP8266 microcontroller boards. The system can be used to collect and transmit sensor data in situations where it would be too difficult or energy-consuming to provide full WiFi coverage.

FDRS devices are divided into two types. Gateways comprise the infrastructure of the network, moving data in pre-determined directions to provide coverage to all devices. User Nodes allow the user to exchange data with the gateways and can be organized into sensor, controller, or combination nodes.

## Getting Started
### Installation
To install FDRS, download the project folder and move it to your Arduino libraries folder. You will then be able to access all of the FDRS sketch files from the examples menu.

Install the libraries that you need:
- [ArduinoJson](https://arduinojson.org/) (mandatory)
- [LoRa library](https://github.com/sandeepmistry/arduino-LoRa) by sandeepmistry (required if using LoRa)
- [PubSubClient](https://github.com/knolleary/pubsubclient/) (required for MQTT Gateways)
- [ArduinoUniqueID](https://github.com/ricaun/ArduinoUniqueID) (required for LoRa sensors/controllers)

### User nodes
**[Node Documentation](/extras/Node.md)**

Nodes can be described as *sensors, controllers, or both*:
- A **Sensor node** aggregates data into a packet, then sends it to a gateway via ESP-NOW or LoRa.
- A **Controller node** registers with a gateway to begin receiving data from it, then subscribes to the specific READING_ID(s) that it will listen for data from. When data arrives from an ID the device is subscribed to, a callback function is activated containing the data.
  
### Gateways
**[Gateway Documentation](extras/Gateway.md)**
  
Gateways listen for packets over ESP-NOW, LoRa, UART, and/or MQTT, then re-transmit the packets using one or more of the same interfaces.
  
### Front-end
You can access your data using a Raspberry Pi (or other computer) linked to your final FDRS Gateway via either UART (serial-to-USB) or MQTT. Node-RED is my favorite platform for accessing/manipulating data on the front-end, and InfluxDB+Grafana is the dream team for storage and visualization. 


## Future Plans
 A few things that I intend to add are:
- More sensor and controller sketches! If you have designed any open source hardware for ESP32 or 8266, please contact me and I will provide support for your device in this repo.
- Support for several new devices and protocols: ethernet, nRF24L01, 4G LTE, and the E5 LoRa module from Seeed Studio.
- Channel Activity Detection (CAD) and some improvements for LoRa, including controller capabilities.
 
## Thank you
**...very much for checking out my project!** I truly appreciate everyone who has reached out with contributions and assistance, especially those featured in the "Contributors" section. If you have any questions, comments, issues, or suggestions please don't heasitate to contact me at timmbogner@gmail.com or open a discussion here on Github.

Winter is coming, which means my budget will be quite thin. **If you are having fun with FDRS, [please consider supporting me](https://www.buymeacoffee.com/TimmB) so that I can spend more time building it.**

Many thanks go to the ever-instructional [**Andreas Spiess**](https://www.youtube.com/channel/UCu7_D0o48KbfhpEohoP7YSQ). His insight and ideas took this project from a roughly-hewn stone to the "[diamond](https://youtu.be/6JI5wZABWmA)" you see today. 

It is a great honor to have been [featured on **Hackaday**!](https://hackaday.com/2022/07/02/farm-data-relay-system/)
  
[**Random Nerd Tutorials**](https://randomnerdtutorials.com/) was also an indispensable source of ESP knowledge. If you are a beginner and trying to learn more about   microcontrollers, I highly reccomend starting there.
  

Development of this project would not have been possible without the support of my former employer, **Sola Gratia Farm** of **Urbana, IL, USA**.  Sola Gratia is a community-based farm dedicated to growing high-quality produce and sharing it with those in need. Thank you.
  

  
![Basic](extras/Basic_Setup.png)
![Advanced](extras/Advanced_Setup.png)
