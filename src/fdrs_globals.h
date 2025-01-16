// FARM DATA RELAY SYSTEM

// Global Configuration

// Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
#ifndef __FDRS_GLOBALS_h__
#define __FDRS_GLOBALS_h__

#define GLOBAL_DBG_LEVEL 0


#define GLOBAL_WIFI_SSID        "Your SSID"
#define GLOBAL_WIFI_PASS        "Password"

#define GLOBAL_DNS1_IPADDRESS    "8.8.8.8"   // Default to Google Primary DNS
#define GLOBAL_DNS2_IPADDRESS    "8.8.4.4"   // Default to Google Secondary DNS

#define GLOBAL_MQTT_ADDR "192.168.0.8"
#define GLOBAL_MQTT_PORT 1883

//#define GLOBAL_MQTT_AUTH   //uncomment to enable MQTT authentication  
#define GLOBAL_MQTT_USER   "Your MQTT Username"
#define GLOBAL_MQTT_PASS   "Your MQTT Password"
// MQTT Topics
#define GLOBAL_TOPIC_DATA    "fdrs/data"
#define GLOBAL_TOPIC_STATUS  "fdrs/status"
#define GLOBAL_TOPIC_COMMAND "fdrs/command"

// NTP Time Server
#define GLOBAL_DST_RULE        USDST    // Daylight saving time rules: Use USDST for United States DST rules, EUDST for European Union
#define GLOBAL_TIME_SERVER      "0.us.pool.ntp.org"
#define GLOBAL_STD_OFFSET      (-6)                // Local standard time offset in hours from UTC - if unsure, check https://time.is
#define GLOBAL_DST_OFFSET      (STD_OFFSET + 1)    // Local savings time offset in hours from UTC - if unsure, check https://time.is
#define GLOBAL_TIME_FETCHNTP    60      // Time in minutes between fetching time from NTP server
#define GLOBAL_TIME_PRINTTIME   15      // Time in minutes between printing local time
#define GLOBAL_TIME_SEND_INTERVAL 60    // Time in minutes between sending out time

#define GLOBAL_LORA_FREQUENCY 915.0   // Carrier frequency in MHz. Allowed values range from 137.0 MHz to 1020.0 MHz (varies by chip).
#define GLOBAL_LORA_SF 7     // LoRa link spreading factor. Allowed values range from 6 to 12.
#define GLOBAL_LORA_BANDWIDTH 125.0  // LoRa link bandwidth in kHz. Allowed values are 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250 and 500 kHz.
#define GLOBAL_LORA_CR 5    // LoRa link coding rate denominator. Allowed values range from 5 to 8.
#define GLOBAL_LORA_SYNCWORD 0x12    // LoRa sync word. Can be used to distinguish different LoRa networks. Note that 0x34 is reserved for LoRaWAN.
#define GLOBAL_LORA_INTERVAL 5000  // Interval between LoRa buffer releases. Must be longer than transmission time-on-air.

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // MAC address prefix. Can be used to distinguish different ESP-NOW networks.

#if defined(USE_OLED) || defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
    #define USE_I2C
#endif

#if defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
  #define USE_RTC
#endif

#if defined(USE_GPS) && defined(ESP8266)
  #error "For ESP8266 only one UART has both Tx and Rx capabilities. GPS not supported for ESP8266"
#endif

#if defined(USE_ETHERNET) && !defined(ESP32)
  #error "Ethernet only supported for ESP32."
#endif

#if defined(USE_OLED) && (!defined(ESP32) && !defined(ESP8266))
  #warning "OLED current supported for only ESP32 or ESP8266."
  #undef USE_OLED
#endif

#endif // __FDRS_GLOBALS_h__
