// FARM DATA RELAY SYSTEM

// Global Configuration

// Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.


#define GLOBAL_WIFI_SSID "Your SSID"
#define GLOBAL_WIFI_PASS "Password"

#define GLOBAL_MQTT_ADDR "192.168.0.8"
#define GLOBAL_MQTT_PORT 1883

//#define GLOBAL_MQTT_AUTH   //uncomment to enable MQTT authentication  
#define GLOBAL_MQTT_USER   "Your MQTT Username"
#define GLOBAL_MQTT_PASS   "Your MQTT Password"
// MQTT Topics
#define TOPIC_DATA    "fdrs/data"
#define TOPIC_STATUS  "fdrs/status"
#define TOPIC_COMMAND "fdrs/command"
#define TOPIC_DATA_BACKLOG "fdrs/databacklog"   // Used in filesystem module

#define GLOBAL_LORA_FREQUENCY 915.0   // Carrier frequency in MHz. Allowed values range from 137.0 MHz to 1020.0 MHz (varies by chip).
#define GLOBAL_LORA_SF 7     // LoRa link spreading factor. Allowed values range from 6 to 12.
#define GLOBAL_LORA_BANDWIDTH 125.0  // LoRa link bandwidth in kHz. Allowed values are 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250 and 500 kHz.
#define GLOBAL_LORA_CR 5    // LoRa link coding rate denominator. Allowed values range from 5 to 8.
#define GLOBAL_LORA_SYNCWORD 0x12    // LoRa sync word. Can be used to distinguish different LoRa networks. Note that 0x34 is reserved for LoRaWAN.
#define GLOBAL_LORA_INTERVAL 5000  // Interval between LoRa buffer releases. Must be longer than transmission time-on-air.

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // MAC address prefix. Can be used to distinguish different ESP-NOW networks.