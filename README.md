# <p align="center">Farm Data Relay System
##### <p align="center">[***In loving memory of Gay Holman, an extraordinary woman.***](https://www.facebook.com/CFECI/posts/2967989419953119) #####

The Farm Data Relay System is an easy way to link remote sensors to the internet without the need for WiFi. It is based around the ESP-NOW protocol, which is readily available on ESP32 and ESP8266 microcontroller boards. The system can be used to collect and transmit sensor data in situations where it would be too difficult or energy-consuming to provide full WiFi coverage. 

Using an assigned MAC address scheme allows for the whole system to be configured by setting just a handful of values in code.  Every ESP-NOW gateway is assigned a single-byte identifier, known as the UNIT_MAC. This along with a set, 5-byte prefix is assigned to the MAC address of the ESP's radio at boot. 

Gateways can be configured to send an ESP-NOW transmission either to the serial port using JSON, another ESP-NOW gateway, or broadcast it via LoRa PHY. An incoming transmission from the serial port can also be routed to the same interfaces, with the addition of MQTT.

## Getting Started
To use FDRS with Node-Red and MQTT you'll need two ESP devices (gateways) that are connected via UART, plus additional ESP devices with sensors connected.
The two **gateways** are programmed using the instructions [found with the Gateway2000 sketch](https://github.com/timmbogner/Farm-Data-Relay-System/tree/main/FDRS_Gateway2000). 
The **sensors** can either use the [example sketches](https://github.com/timmbogner/Farm-Data-Relay-System/tree/main/Sensors) included, or you can use the “fdrs_sensor.h” file to [use FDRS with a sketch you’ve already written.](https://github.com/timmbogner/Farm-Data-Relay-System/tree/main/FDRS_Sensor2000)
![Basic](/FDRS_Gateway2000/Basic_Setup.png)
### Sensors
```
typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;
} DataReading;
```
Each sensor in the system sends its data over ESP-NOW as a float 'd' inside of a structure called a DataReading. Its global sensor address is represented by an integer 'id', and each type is represented by a single byte 't'.  If sensors need to send multiple types of readings (ex: temp and humidity), then they are sent in an array of DataReadings. A single DataReading.id may have multiple readings of different types associated with it. 

## Thank you
...very much for checking out my project! I truly appreciate everyone across the net who has reached out with assistance and encouragement. If you have any questions, comments, or issues please feel free to contact me at bogner1@gmail.com.

If you have any extra money laying around, you could [send it to me via this Paypal link](https://www.paypal.com/donate/?business=F2MYGWWTGG5PN&no_recurring=0&item_name=Anything+helps%21&currency_code=USD). I'm a farmer by occupation, and donations would help me to spend more time developing farm gadgets over the winter months. 

Development of this project would not have been possible without the support of my employer, [***Sola Gratia Farm***](https://www.solagratiacsa.com/) of **Urbana, IL, USA**.  Sola Gratia is a community-based farm dedicated to growing high-quality, natural produce and sharing it with those in need. Thank you!
  
A huge thanks to the ever-instructional [**Andreas Spiess**](https://www.youtube.com/channel/UCu7_D0o48KbfhpEohoP7YSQ).
  
[**Random Nerd Tutorials**](https://randomnerdtutorials.com/) is also an indispensable source of ESP knowledge.
