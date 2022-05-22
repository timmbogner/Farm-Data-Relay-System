//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Configuration

#define UNIT_MAC     0x00  // The address of this gateway

//Actions -- Define what happens when a packet arrives at each interface:
//Current function options are: sendESPNOW(MAC), sendSerial(), sendMQTT(), bufferESPNOW(interface), bufferSerial(), and bufferLoRa(interface).

#define DEBUG
#define CREDENTIALS

//#define ESPNOWG_ACT    sendSerial();
// #define SERIAL_ACT
// #define MQTT_ACT
#define LORAG_ACT      sendSerial();

#define USE_LORA
//#define USE_WIFI    //Used only for MQTT gateway

#define CREDENTIALS

#if defined (ESP32)
#define RXD2 14
#define TXD2 15
#define UART_IF Serial2
#else
#define UART_IF Serial
#endif

//LoRa Configuration -- Needed only if this device is using LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
