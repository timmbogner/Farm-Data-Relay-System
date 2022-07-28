/*  FARM DATA RELAY SYSTEM
*
*  "fdrs_globals.h"
*
*  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
*/

#ifndef FDRS_GLOBALS
#define FDRS_GLOBALS

#define GLOBAL_SSID "Your SSID"
#define GLOBAL_PASS "Password"

#define GLOBAL_MQTT_ADDR "192.168.0.8"
#define GLOBAL_MQTT_PORT 1883

//#define GLOBAL_MQTT_AUTH   //Enable MQTT authentication 
#define GLOBAL_MQTT_USER   "Your MQTT Username"
#define GLOBAL_MQTT_PASS   "Your MQTT Password"

#define GLOBAL_LORA_BAND 915E6      // LoRa Frequency Band
#define GLOBAL_LORA_SF 7            // LoRa Spreading Factor
#define GLOBAL_LORA_TXPWR 17        // LoRa TX power in dBm (+2dBm - +20dBm), default is +17dBm.  Lower power = less battery use
#define GLOBAL_LORA_ACK_TIMEOUT 400   // ms timeout waiting for LoRa ACKs (if enabled).  Wouldn't go less than 200ms
#define GLOBAL_LORA_RETRIES 2

#endif //FDRS_GLOBALS
