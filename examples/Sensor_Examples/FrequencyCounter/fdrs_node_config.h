//  FARM DATA RELAY SYSTEM
//
//  Sensor Configuration


#define READING_ID    23   //Unique ID for this sensor
#define GTWY_MAC      0x01 //Address of the nearest gateway

#define USE_ESPNOW
//#define USE_LORA
#define DEEP_SLEEP
#define POWER_CTRL    22

#define FDRS_DEBUG
//#define DBG_LEVEL 0    // 0 for minimal messaging, 1 for troubleshooting, 2 for development

// I2C - OLED or RTC
#define I2C_SDA 5
#define I2C_SCL 6

// OLED -- Displays console debugging messages on an SSD1306 I²C OLED
// #define USE_OLED    
#define OLED_HEADER "FDRS"
#define OLED_PAGE_SECS 30
#define OLED_RST -1

// LoRa Configuration
#define RADIOLIB_MODULE SX1276 //Tested on SX1276
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO 26
#define LORA_BUSY  33
//#define USE_SX126X

//#define CUSTOM_SPI
#define LORA_SPI_SCK  5
#define LORA_SPI_MISO 19
#define LORA_SPI_MOSI 27

#define LORA_TXPWR 17    // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))
#define LORA_ACK        // Request LoRa acknowledgment.

// Time settings
#define USDST
// #define EUDST
#define STD_OFFSET      (-6)                // Local standard time offset in hours from UTC - if unsure, check https://time.is
#define DST_OFFSET      (STD_OFFSET + 1)    // Local savings time offset in hours from UTC - if unsure, check https://time.is
#define TIME_PRINTTIME    15     // Time, in minutes, between printing local time to debug