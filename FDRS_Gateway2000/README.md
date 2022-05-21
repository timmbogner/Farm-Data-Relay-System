# Gateway 2.000

This is the FDRS Multiprotocol Gateway sketch. The device listens for packets over ESP-NOW, UART, LoRa, and/or MQTT, then retransmits the packets over these interfaces using rules defined in the "Actions" section of the configuration file.

The most commonly used configuration tells the device to take any ESP-NOW packet it receives and output the data over the serial port (UART):
```
#define UNIT_MAC       0x00
#define ESPNOWG_ACT    sendSerial();
```
The companion for this device, connected via serial, takes any data it receives from the serial port and sends it via MQTT:
```
#define USE_WIFI
#define SERIAL_ACT   sendMQTT();
```
Splitting the gateway into two devices allows you to use ESP-NOW and WiFi simultaneously without channel conflicts. You can also connect the first device to a computer with a USB-UART adapter and get the data that way, eliminating WiFi altogether.

If you have sensors that are out of range of your first gateway, you can use a gateway as a repeater. First set the UNIT_MAC to 0x01, then send general ESP-NOW traffic to the address of the first gateway:
```
#define UNIT_MAC     0x01 
#define ESPNOWG_ACT  sendESPNOW(0x00);
```
### LoRa
You can also use LoRa to expand the distances between hops. While ESP-NOW is quick enough to handle a lot of traffic in real-time, LoRa is much slower. For this reason, you must send LoRa data to a buffer and transmit it at standard intervals. 



![Basic](/FDRS_Gateway2000/Basic_Setup.png)

![Advanced](/FDRS_Gateway2000/Advanced_Setup.png)

![Basic LoRa](/FDRS_Gateway2000/Basic_LoRa_Setup.png)

![Advanced LoRa](/FDRS_Gateway2000/Advanced_Setup_LoRa.png)

