# <p align="center">Farm Data Relay System
##### <p align="center">[***In loving memory of Gay Holman, an extraordinary woman.***](https://www.facebook.com/CFECI/posts/2967989419953119) #####

The Farm Data Relay System is an easy way to link remote sensors to the internet without the need for WiFi. It is based around the ESP-NOW protocol, which is readily available on ESP32 and ESP8266 microcontroller boards. The system can be used to collect and transmit sensor data in situations where it would be too difficult or energy-consuming to provide full WiFi coverage. 

Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code.  Every ESP-NOW gateway is assigned a single-byte identifier, known as the UNIT_MAC. This along with a set, 5-byte prefix is assigned to the MAC address of the ESP's radio at boot. 

Gateways can be configured to send an ESP-NOW transmission either to the serial port using JSON, another ESP-NOW gateway, or broadcast it via LoRa PHY. An incoming transmission from the serial port can also be routed to the same interfaces, with the addition of MQTT.

## Getting Started
To use FDRS with Node-Red and MQTT you'll need two ESP devices (gateways) that are connected via UART, plus additional ESP devices with sensors connected.

  **Sensors** can use the [“fdrs_sensor.h”](https://github.com/timmbogner/Farm-Data-Relay-System/tree/main/FDRS_Sensor2000) file to set up the device and send FDRS data via ESP-NOW and/or LoRa. 
  
The two **gateways** are programmed using the instructions [found with the Gateway2000 sketch](https://github.com/timmbogner/Farm-Data-Relay-System/tree/main/FDRS_Gateway2000).
 
The Node-RED **front-end** can be set up with these nodes:
  ```
[{"id":"66d36c0f.cedf94","type":"influxdb out","z":"d7346a99.716ef8","influxdb":"905dd357.34717","name":"","measurement":"DataReading","precision":"","retentionPolicy":"","database":"database","precisionV18FluxV20":"ms","retentionPolicyV18Flux":"","org":"the_organization","bucket":"bkt","x":760,"y":240,"wires":[]},{"id":"93e9822a.3ad59","type":"mqtt in","z":"d7346a99.716ef8","name":"","topic":"esp/fdrs","qos":"2","datatype":"auto","broker":"c513f7e9.760658","nl":false,"rap":true,"rh":0,"x":170,"y":220,"wires":[["d377f9e0.faef98"]]},{"id":"d377f9e0.faef98","type":"json","z":"d7346a99.716ef8","name":"","property":"payload","action":"obj","pretty":false,"x":290,"y":220,"wires":[["ca383562.4014e8"]]},{"id":"ca383562.4014e8","type":"split","z":"d7346a99.716ef8","name":"","splt":"\\n","spltType":"str","arraySplt":1,"arraySpltType":"len","stream":false,"addname":"","x":410,"y":220,"wires":[["6eaba8dd.429e38"]]},{"id":"6eaba8dd.429e38","type":"function","z":"d7346a99.716ef8","name":"Fields","func":"msg.payload = [{\n    data: msg.payload.data\n},{\n    id: msg.payload.id,\n    type: msg.payload.type\n}]\nreturn msg;","outputs":1,"noerr":0,"initialize":"","finalize":"","libs":[],"x":530,"y":220,"wires":[["296d0f4b.37a46","66d36c0f.cedf94"]]},{"id":"296d0f4b.37a46","type":"debug","z":"d7346a99.716ef8","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"false","statusVal":"","statusType":"auto","x":670,"y":200,"wires":[]},{"id":"905dd357.34717","type":"influxdb","hostname":"127.0.0.1","port":"8086","protocol":"http","database":"database","name":"","usetls":false,"tls":"d50d0c9f.31e858","influxdbVersion":"2.0","url":"http://localhost:8086","rejectUnauthorized":true},{"id":"c513f7e9.760658","type":"mqtt-broker","name":"","broker":"localhost","port":"1883","clientid":"","usetls":false,"protocolVersion":"4","keepalive":"60","cleansession":true,"birthTopic":"","birthQos":"0","birthPayload":"","birthMsg":{},"closeTopic":"","closeQos":"0","closePayload":"","closeMsg":{},"willTopic":"","willQos":"0","willPayload":"","willMsg":{},"sessionExpiry":""},{"id":"d50d0c9f.31e858","type":"tls-config","name":"","cert":"","key":"","ca":"","certname":"","keyname":"","caname":"","servername":"","verifyservercert":false}]
```
  
![Basic](/FDRS_Gateway2000/Basic_Setup.png)
### Sensors
```
typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each sensor in the system sends its data over ESP-NOW as a float 'd' inside of a structure called a DataReading. Its global sensor address is represented by an integer 'id', and each type is represented by a single byte 't'.  If a sensor module needs to send multiple types of readings, then they are sent in an array of DataReadings. A single DataReading.id may have readings of multiple types associated with it.

## Thank you
...very much for checking out my project! I truly appreciate everyone across the net who has reached out with assistance and encouragement. If you have any questions, comments, or issues please feel free to contact me at timmbogner@gmail.com.

If you have any extra money laying around, you could [send it to me via this Paypal link](https://www.paypal.com/donate/?business=F2MYGWWTGG5PN&no_recurring=0&item_name=Anything+helps%21&currency_code=USD). I'm a farmer by occupation, and donations would help me to spend more time developing farm gadgets over the winter months. 

Development of this project would not have been possible without the *gracious* support of my former employer, [**Sola Gratia Farm**](https://www.solagratiacsa.com/) of **Urbana, IL, USA**.  Sola Gratia is a community-based farm dedicated to growing high-quality produce and sharing it with those in need. Thank you!
  
A huge thanks to the ever-instructional [**Andreas Spiess**](https://www.youtube.com/channel/UCu7_D0o48KbfhpEohoP7YSQ).
  
[**Random Nerd Tutorials**](https://randomnerdtutorials.com/) is also an indispensable source of ESP knowledge.
