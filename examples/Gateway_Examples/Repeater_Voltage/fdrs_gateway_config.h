//  FARM DATA RELAY SYSTEM
//
//  GATEWAY CONFIGURATION

//Addresses
#define UNIT_MAC           0x02  // The address of this gateway

#define ESPNOW_NEIGHBOR_1  0x01  // Address of ESP-NOW neighbor #1
#define ESPNOW_NEIGHBOR_2  0x04  // Address of ESP-NOW neighbor #2
#define LORA_NEIGHBOR_1    0x00  // Address of LoRa neighbor #1
#define LORA_NEIGHBOR_2    0x00  // Address of LoRa neighbor #2

// Interfaces
#define USE_ESPNOW  
//#define USE_LORA
//#define USE_WIFI  // Will cause errors if used with ESP-NOW. Use a serial link instead!
//#define USE_ETHERNET

// Routing
// Options: sendESPNowNbr(1 or 2); sendESPNowPeers(); sendLoRaNbr(1 or 2); broadcastLoRa(); sendSerial(); sendMQTT();
#define ESPNOWG_ACT    sendESPNowNbr(1);
#define LORAG_ACT      
#define SERIAL_ACT     
#define MQTT_ACT          
#define INTERNAL_ACT   sendESPNowNbr(1);
#define ESPNOW1_ACT    sendESPNowNbr(2); sendESPNowPeers();
#define ESPNOW2_ACT    sendESPNowNbr(1);                
#define LORA1_ACT      
#define LORA2_ACT 

// LoRa Configuration
#define RADIOLIB_MODULE SX1276
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DIO   26
#define LORA_BUSY  33
//#define USE_SX126X

#define LORA_TXPWR 17   // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))

//#define CUSTOM_SPI
#define LORA_SPI_SCK  5
#define LORA_SPI_MISO 19
#define LORA_SPI_MOSI 27

#define FDRS_DEBUG     // Enable USB-Serial debugging
//#define DBG_LEVEL 0    // 0 for minimal messaging, 1 for troubleshooting, 2 for development

// I2C - OLED or RTC
#define I2C_SDA 4
#define I2C_SCL 15

// OLED -- Displays console debugging messages on an SSD1306 I²C OLED
///#define USE_OLED    
#define OLED_HEADER "FDRS"
#define OLED_PAGE_SECS 30
#define OLED_RST 16

// RTC - I2C
// #define USE_RTC_DS3231
// #define RTC_ADDR 0x57
// #define USE_RTC_DS1307
// #define RTC_ADDR 0x68

// UART data interface pins (if available)
#define RXD2 14
#define TXD2 15

//#define USE_LR  // Use ESP-NOW LR mode (ESP32 only)

// WiFi and MQTT Credentials  -- These will override the global settings
//#define WIFI_SSID   "Your SSID"  
//#define WIFI_PASS   "Your Password"

//#define MQTT_ADDR   "192.168.0.8"
//#define MQTT_PORT   1883 // Default MQTT port is 1883
//#define MQTT_AUTH   //Enable MQTT authentication 
//#define MQTT_USER   "Your MQTT Username"
//#define MQTT_PASS   "Your MQTT Password"

// NTP Time settings
#define USDST
// #define EUDST
#define TIME_SERVER       "0.us.pool.ntp.org"       // NTP time server to use. If FQDN at least one DNS server is required to resolve name
#define STD_OFFSET      (-6)                // Local standard time offset in hours from UTC - if unsure, check https://time.is
#define DST_OFFSET      (STD_OFFSET + 1)    // Local savings time offset in hours from UTC - if unsure, check https://time.is
#define TIME_FETCHNTP     60      // Time, in minutes, between fetching time from NTP server
#define TIME_PRINTTIME    15     // Time, in minutes, between printing local time to debug
#define TIME_SEND_INTERVAL 10    // Time, in minutes, between sending out time to remote devices