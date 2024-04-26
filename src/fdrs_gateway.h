//  FARM DATA RELAY SYSTEM
//
//  GATEWAY Main Functions
//  Developed by Timm Bogner (timmbogner@gmail.com)

#include "fdrs_datatypes.h"
#include "fdrs_globals.h"
#define FDRS_GATEWAY

#ifndef ESPNOWG_ACT
#define ESPNOWG_ACT
#endif
#ifndef ESPNOW1_ACT
#define ESPNOW1_ACT
#endif
#ifndef ESPNOW2_ACT
#define ESPNOW2_ACT
#endif
#ifndef SERIAL_ACT
#define SERIAL_ACT
#endif
#ifndef MQTT_ACT
#define MQTT_ACT
#endif
#ifndef LORAG_ACT
#define LORAG_ACT
#endif
#ifndef LORA1_ACT
#define LORA1_ACT
#endif
#ifndef LORA2_ACT
#define LORA2_ACT
#endif
#ifndef INTERNAL_ACT
#define INTERNAL_ACT
#endif
#ifdef USE_ETHERNET
#ifndef USE_WIFI
#define USE_WIFI
#endif
#endif // USE_ETHERNET

SystemPacket theCmd;
DataReading theData[256];
TimeSource timeSource;
uint8_t ln;
uint8_t newData = event_clear;
uint8_t newCmd = cmd_clear;

DataReading fdrsData[256]; // buffer for loadFDRS()
uint8_t data_count = 0;

// Function Prototypes needed due to #ifdefs being moved outside of function definitions in header files 
void broadcastLoRa();
void sendLoRaNbr(uint8_t);
void timeFDRSLoRa(uint8_t *);
//static uint16_t crc16_update(uint16_t, uint8_t);
void sendESPNowNbr(uint8_t);
void sendESPNowPeers();
void sendESPNow(uint8_t);
void sendTimeSerial();
void handleLoRa();
void handleMQTT();
void handleOTA();

void sendMQTT();
void sendLog();
void resendLog();
void releaseLogBuffer();
void printFDRS(DataReading*, int);

#ifdef USE_I2C
  #include <Wire.h>
#endif
#ifdef USE_OLED
  #include "fdrs_oled.h"
#endif
#include "fdrs_debug.h"
#include "fdrs_time.h"
#include "fdrs_gateway_serial.h"
#include "fdrs_gateway_scheduler.h"
#ifdef USE_ESPNOW
  #include "fdrs_gateway_espnow.h"
#endif
#ifdef USE_LORA
  #include "fdrs_lora.h"
#endif
#ifdef USE_WIFI
  #include "fdrs_gateway_wifi.h"
  #include "fdrs_gateway_mqtt.h"
#include "fdrs_gateway_ota.h"
#endif

#ifdef DEBUG_CONFIG
  #include "fdrs_checkConfig.h"
#endif


// Print type DataReading for debugging purposes
void printFDRS(DataReading * dr, int len) {
  DBG("----- printFDRS: " + String(len) + " records -----");
  for(int i = 0; i < len; i++) {
    DBG("Index: " + String(i) + "| id: " + String(dr[i].id) + "| type: " + String(dr[i].t) + "| data: " + String(dr[i].d));
  }
  DBG("----- End printFDRS -----");

}

void sendFDRS()
{
  if(data_count > 0) {
    for (int i = 0; i < data_count; i++)
    {
      theData[i].id = fdrsData[i].id;
      theData[i].t = fdrsData[i].t;
      theData[i].d = fdrsData[i].d;
    }
    ln = data_count;
    data_count = 0;
    newData = event_internal;
    DBG("Entered internal data.");
  }
}

void loadFDRS(float d, uint8_t t, uint16_t id)
{
  // guard against buffer overflow
  if(data_count > 253) {
    sendFDRS();
  }
  DBG("Id: " + String(id) + " - Type: " + String(t) + " - Data loaded: " + String(d));
  DataReading dr;
  dr.id = id;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}

void beginFDRS()
{
#if defined(ESP8266)
  Serial.begin(115200);
#elif defined(ESP32)
  Serial.begin(115200);
  UART_IF.begin(115200, SERIAL_8N1, RXD2, TXD2);
#endif
#ifdef USE_GPS
  begin_gps();
#endif
#ifdef USE_I2C
  Wire.begin(I2C_SDA, I2C_SCL);
#endif
#ifdef USE_RTC
  begin_rtc();
#endif

#ifdef USE_OLED
  init_oled();
  DBG("Display initialized!");
  DBG("Hello, World!");
#endif
  DBG("");
  DBG("Initializing FDRS Gateway!");
  DBG("Address: " + String(UNIT_MAC, HEX));
#ifdef USE_ESPNOW
  DBG1("ESPNOW_NEIGHBOR_1: " + String(ESPNOW_NEIGHBOR_1, HEX));
  DBG1("ESPNOW_NEIGHBOR_2: " + String(ESPNOW_NEIGHBOR_2, HEX));
#endif // USE_ESPNOW
#ifdef USE_LORA
  DBG1("LORA_NEIGHBOR_1: " + String(LORA_NEIGHBOR_1, HEX));
  DBG1("LORA_NEIGHBOR_2: " + String(LORA_NEIGHBOR_2, HEX));
#endif // USE_LORA
  DBG("Debugging verbosity level: " + String(DBG_LEVEL));

#ifdef USE_LORA
  begin_lora();
  #endif
#ifdef USE_WIFI
  begin_wifi();
  DBG("Connected.");
  begin_mqtt();
  begin_ntp();
  begin_OTA();
#endif
#ifdef USE_ESPNOW
  begin_espnow();
#endif


#ifdef USE_WIFI
  client.publish(TOPIC_STATUS, "FDRS initialized");
  scheduleFDRS(fetchNtpTime,1000*60*FDRS_TIME_FETCHNTP);
#endif
scheduleFDRS(printTime,1000*60*FDRS_TIME_PRINTTIME);
}

void handleCommands()
{
  switch (theCmd.cmd)
  {
  case cmd_ping:
#ifdef USE_ESPNOW
    pingback_espnow();
#endif // USE_ESPNOW

    break;

  case cmd_add:
#ifdef USE_ESPNOW
    add_espnow_peer();
#endif // USE_ESPNOW

    break;

  case cmd_time:
    if(theCmd.param > MIN_TS) {
#ifdef USE_ESPNOW
      recvTimeEspNow(theCmd.param);
#endif // USE_ESPNOW
    }
    else if(theCmd.param == 0) {
#ifdef USE_ESPNOW
      sendTimeESPNow(incMAC);
#endif // USE_ESPNOW
    }
    
    break;  
  }
  theCmd.cmd = cmd_clear;
  theCmd.param = 0;
}

void handleActions() {
  if (newData != event_clear)
  {
    switch (newData)
    {
    case event_espnowg:
      ESPNOWG_ACT
      break;
    case event_espnow1:
      ESPNOW1_ACT
      break;
    case event_espnow2:
      ESPNOW2_ACT
      break;
    case event_serial:
      SERIAL_ACT
      break;
    case event_mqtt:
      MQTT_ACT
      break;
    case event_lorag:
      LORAG_ACT
      break;
    case event_lora1:
      LORA1_ACT
      break;
    case event_lora2:
      LORA2_ACT
      break;
    case event_internal:
      INTERNAL_ACT
      break;
    }
    newData = event_clear;
  }
}


void loopFDRS()
{
  handleTime();
  handle_schedule();
  handleCommands();
  handleSerial();
  handleLoRa();
  handleMQTT();
  handleOTA();
#ifdef USE_OLED
  drawPageOLED(true);
#endif
  handleActions();
}

// "Skeleton Functions related to FDRS Actions"
#ifndef USE_LORA
  void broadcastLoRa() {}
  void sendLoRaNbr(uint8_t address) {}
  void timeFDRSLoRa(uint8_t *address) {}  // fdrs_gateway_lora.h
  void sendTimeLoRa() { return; }                  // fdrs_gateway_time.h
  void handleLoRa() { return; }                    // fdrs_gateway_lora.h
  bool pingLoRaTimeSource() { return false; } //fdrs_gateway_lora.h
#endif
#ifndef USE_ESPNOW
  void sendESPNowNbr(uint8_t interface) { }
  void sendESPNowPeers() { }
  void sendESPnow(uint8_t address) { }
  esp_err_t sendTimeESPNow() { return ESP_OK; }                  // fdrs_gateway_time.h
#endif
#ifndef USE_WIFI
  void sendMQTT() {}
  void handleMQTT() {}
  void handleOTA() {}
#endif