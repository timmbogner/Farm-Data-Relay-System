# <p align="center">Farm Data Relay System
##### <p align="center">[***In loving memory of Gay Holman, an extraordinary woman.***](https://www.facebook.com/CFECI/posts/2967989419953119) #####

The Farm Data Relay System is an easy way to collect data from remote sensors without relying on WiFi. It is based around the ESP-NOW protocol, which is readily available on ESP32 and ESP8266 microcontroller boards. The system can be used to collect and transmit sensor data in situations where it would be too difficult or energy-consuming to provide full WiFi coverage. 

Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code. Every wireless gateway is assigned a single-byte identifier, known as the UNIT_MAC. This along with a set, 5-byte prefix, is assigned to the MAC address of the ESP at boot. 

## Getting Started
### Installation
To install FDRS, download the project folder and move it to your Arduino libraries folder. You will then be able to access all of the FDRS sketch files from the examples menu.

Install the libraries that you need:
- [ArduinoJson](https://arduinojson.org/) (mandatory)
- [LoRa library](https://github.com/sandeepmistry/arduino-LoRa) by sandeepmistry (required if using LoRa)
- [PubSubClient](https://github.com/knolleary/pubsubclient/) (required for MQTT Gateways)
- [ArduinoUniqueID](https://github.com/ricaun/ArduinoUniqueID) (required for LoRa sensors/controllers)

### [Sensors](/extras/Sensor.md)
Sensors gather data and send it to a gateway via ESP-NOW or LoRa. 
  
### [Gateways](extras/Gateway.md)
Gateways listen for packets over ESP-NOW, LoRa, UART, and/or MQTT, then re-transmit the packets using one or more of the same interfaces.
  
 ### Front-end
The Node-RED front-end can be set up with these nodes to format and send the data to InfluxDB:
  ```
[{"id":"66d36c0f.cedf94","type":"influxdb out","z":"d7346a99.716ef8","influxdb":"905dd357.34717","name":"","measurement":"DataReading","precision":"","retentionPolicy":"","database":"database","precisionV18FluxV20":"ms","retentionPolicyV18Flux":"","org":"the_organization","bucket":"bkt","x":760,"y":240,"wires":[]},{"id":"93e9822a.3ad59","type":"mqtt in","z":"d7346a99.716ef8","name":"","topic":"esp/fdrs","qos":"2","datatype":"auto","broker":"c513f7e9.760658","nl":false,"rap":true,"rh":0,"x":170,"y":220,"wires":[["d377f9e0.faef98"]]},{"id":"d377f9e0.faef98","type":"json","z":"d7346a99.716ef8","name":"","property":"payload","action":"obj","pretty":false,"x":290,"y":220,"wires":[["ca383562.4014e8"]]},{"id":"ca383562.4014e8","type":"split","z":"d7346a99.716ef8","name":"","splt":"\\n","spltType":"str","arraySplt":1,"arraySpltType":"len","stream":false,"addname":"","x":410,"y":220,"wires":[["6eaba8dd.429e38"]]},{"id":"6eaba8dd.429e38","type":"function","z":"d7346a99.716ef8","name":"Fields","func":"msg.payload = [{\n    data: msg.payload.data\n},{\n    id: msg.payload.id,\n    type: msg.payload.type\n}]\nreturn msg;","outputs":1,"noerr":0,"initialize":"","finalize":"","libs":[],"x":530,"y":220,"wires":[["296d0f4b.37a46","66d36c0f.cedf94"]]},{"id":"296d0f4b.37a46","type":"debug","z":"d7346a99.716ef8","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"false","statusVal":"","statusType":"auto","x":670,"y":200,"wires":[]},{"id":"905dd357.34717","type":"influxdb","hostname":"127.0.0.1","port":"8086","protocol":"http","database":"database","name":"","usetls":false,"tls":"d50d0c9f.31e858","influxdbVersion":"2.0","url":"http://localhost:8086","rejectUnauthorized":true},{"id":"c513f7e9.760658","type":"mqtt-broker","name":"","broker":"localhost","port":"1883","clientid":"","usetls":false,"protocolVersion":"4","keepalive":"60","cleansession":true,"birthTopic":"","birthQos":"0","birthPayload":"","birthMsg":{},"closeTopic":"","closeQos":"0","closePayload":"","closeMsg":{},"willTopic":"","willQos":"0","willPayload":"","willMsg":{},"sessionExpiry":""},{"id":"d50d0c9f.31e858","type":"tls-config","name":"","cert":"","key":"","ca":"","certname":"","keyname":"","caname":"","servername":"","verifyservercert":false}]
```

## Future Plans
 A few things that I intend to add are:
- The next generation of FDRS will define a second type of packet, used to communicate between FDRS devices. With this new type of packet, a sensor will be able to seek out the nearest gateway and send to it. This will also allow [controller devices](https://github.com/timmbogner/Farm-Data-Relay-System/tree/main/Controllers) to register with and receive packets from gateways. Exciting stuff!
- The ability to send data in reverse, and have nodes to control irrigation, ventilation, and LED illumination. This will be achieved using the pairing technique above.
- More sensor sketches! If you have designed any open source sensor modules for ESP32 or 8266, please contact me and I will provide a link and/or code for your device in this repo.
- Support for several new devices and protocols: ethernet, nRF24L01, 4G LTE, and the E5 LoRa module from Seeed Studio.
- Some ability to compress data for more efficient LoRa usage and to avoid using floats. Better documentation/development of the DataReading 'type' attribute will come with this. 
 
## Thank you
**...very much for checking out my project!** I truly appreciate everyone across the net who has reached out with assistance and encouragement. If you have any questions, comments, or issues please feel free to contact me at timmbogner@gmail.com. If you have the means, **[please consider supporting me](https://www.buymeacoffee.com/TimmB).** I'm a farmer and landscaper by occupation, and donations would help me to spend more time developing farm gadgets in the off-season. 

Many thanks go to the ever-instructional [**Andreas Spiess**](https://www.youtube.com/channel/UCu7_D0o48KbfhpEohoP7YSQ). His insight and ideas took this project from a roughly-hewn stone to the "[diamond](https://youtu.be/6JI5wZABWmA)" you see today. 

It is a great honor to have been [featured on **Hackaday**!](https://hackaday.com/2022/07/02/farm-data-relay-system/)
  
[**Random Nerd Tutorials**](https://randomnerdtutorials.com/) was also an indispensable source of ESP knowledge. If you are a beginner and trying to learn more about   microcontrollers, I highly reccomend starting there.
  
  
Development of this project would not have been possible without the support of my former employer, **Sola Gratia Farm** of **Urbana, IL, USA**.  Sola Gratia is a community-based farm dedicated to growing high-quality produce and sharing it with those in need. Thank you.
  

  
![Basic](extras/Basic_Setup.png)
![Advanced](extras/Advanced_Setup.png)
