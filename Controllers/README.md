# Controllers
FDRS has some ability to communicate bi-directionally, meaning you can control water valves on relays or RGB lights with FastLED. This has always been a side-project and has a few shortcomings, so I don't include it in the main documentation. I do have a better method in mind, which I will roll out sometime before the 2022 holiday season.

Controlling devices has always been kept in mind while designing this system. The simplest (but problematic) technique is to use the DataReading.id as the device's address and have the device check every packet it receives for an 'id' that matches its own. In this way, you just need to forward all traffic to this device and let it sort out the rest.
To send a packet from the MQTT gateway to an LED sketch for instance, start by adding an MQTT action to the first gateway:
```#define MQTT_ACT sendSerial();```. The gateway will now subscribe to 'fdrs/data' and send any data it receives to the serial port.
Next, you will set the first ESP-NOW gateway to transmit all serial data to broadcast ESP-NOW: ```#define SERIAL_ACT bufferESPNOW(0);``` This is a somewhat hidden method of sending a broadcast packet using FDRS. If there are other gateways listening for general ESP-NOW packets they will treat these packets as incoming sensor readings, thus this is an imperfect method for larger setups. If anyone knows how to tell an ESP-NOW broadcast packet from a packet that was specifically addressed to the device, please contact me! 

## The Future
The next iteration of FDRS will involve a method for devices to send a "ping packet" and connect to the responding gateway that has the best response time. Next, the node may send a "pair packet" and the gateway will add it to a list of paired MAC addresses. There will then be a command for the gateway to send data to all paired devices. 
