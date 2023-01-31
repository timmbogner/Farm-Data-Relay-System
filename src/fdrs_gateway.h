//  FARM DATA RELAY SYSTEM
//
//  GATEWAY Main Functions
//  Developed by Timm Bogner (timmbogner@gmail.com)

#ifndef __FDRS_FUNCTIONS_H__
#define __FDRS_FUNCTIONS_H__
#include "fdrs_datatypes.h"
#include "fdrs_globals.h"

SystemPacket theCmd;
DataReading theData[256];
uint8_t ln;
uint8_t newData = event_clear;
uint8_t newCmd = cmd_clear;
bool is_ping = false;

#include "fdrs_gateway_oled.h"
#include "fdrs_gateway_debug.h"
#include "fdrs_gateway_espnow.h"
#include "fdrs_gateway_lora.h"
#include "fdrs_gateway_wifi.h"
#include "fdrs_gateway_filesystem.h"
#include "fdrs_gateway_mqtt.h"
#include "fdrs_gateway_serial.h"
#include "fdrs_gateway_scheduler.h"
#ifdef DEBUG_CONFIG
#include "fdrs_checkConfig.h"
#endif

void beginFDRS()
{
#if defined(ESP8266)
  Serial.begin(115200);
#elif defined(ESP32)
  Serial.begin(115200);
  UART_IF.begin(115200, SERIAL_8N1, RXD2, TXD2);
#endif
#ifdef USE_OLED
  init_oled();
  DBG("Display initialized!");
  DBG("Hello, World!");
#endif
  DBG("Address:" + String(UNIT_MAC, HEX));
#ifdef USE_LORA
  begin_lora();
  scheduleFDRS(asyncReleaseLoRaFirst, FDRS_LORA_INTERVAL);
#endif
#ifdef USE_WIFI
  begin_wifi();
  DBG("WiFi Connected");
  begin_mqtt();
#else
  begin_espnow();
#endif
#ifdef USE_SD_LOG
  begin_SD();
#endif
#ifdef USE_FS_LOG
  begin_FS();
#endif

#ifdef USE_WIFI
  client.publish(TOPIC_STATUS, "FDRS initialized");
#endif
}

void handleCommands()
{
  switch (theCmd.cmd)
  {
  case cmd_ping:
    pingback_espnow();
    break;

  case cmd_add:
    add_espnow_peer();
    break;
  }
  theCmd.cmd = cmd_clear;
  theCmd.param = 0;
}

void loopFDRS()
{
handle_schedule();
  handleCommands();
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
  handleLogger();
#endif
 handleSerial();
#ifdef USE_LORA
  handleLoRa();
#endif
#ifdef USE_WIFI
 handleMQTT();
#endif

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
    }
    newData = event_clear;
  }
}
#endif //__FDRS_FUNCTIONS_H__
