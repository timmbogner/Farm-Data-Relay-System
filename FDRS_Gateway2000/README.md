# Gateway 2.000

This is the FDRS Multiprotocol Gateway sketch. The device listens for packets over ESP-NOW, UART, LoRa, and/or MQTT, then retransmits the packets over these interfaces using rules defined in the configuration file.

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

If you have sensors that are out of range of your first gateway, you can use a gateway as a repeater. First you will set the ESPNOW1 address to that of your first gateway, then you'll direct general ESP-NOW traffic to the ESPNOW1 interface:
```
#define UNIT_MAC     0x01 
#define ESPNOWG_ACT  sendESPNOW(0x00);
```
