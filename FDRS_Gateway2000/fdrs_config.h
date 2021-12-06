//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration
//  This is still in progress. Stay tuned!

//#define USE_WIFI    //You cannot use ESP-NOW while WiFi is in use
#define WIFI_NET "Your SSID"
#define WIFI_PASS "Password"
#define MQTT_ADDR "192.168.0.8"

#define USE_LORA      //Coming soon

#define UNIT_MAC 0x00// THIS UNIT
#define PREV_MAC 0x01// ESPNOW1 Address 
#define NEXT_MAC 0x02// ESPNOW2 Address

//Actions -- Define what happens when a message arrives. 

#define ESPNOW1_ACT sendESPNOW(2); sendSerial();
#define ESPNOW2_ACT sendESPNOW(1); sendSerial();
#define ESPNOWG_ACT sendSerial();
#define SERIAL_ACT  sendESPNOW(0);
#define MQTT_ACT    sendSerial();

//#define ESPNOW1_ACTION
//#define ESPNOW2_ACTION
//#define ESPNOWG_ACTION
//#define SERIAL_ACTION   sendMQTT();
//#define MQTT_ACTION     sendSerial();
