//  FARM DATA RELAY SYSTEM
//
//  "fdrs_node.h"
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//
#include <fdrs_datatypes.h>
#include <fdrs_globals.h>
#define FDRS_NODE

// CRC16 from https://github.com/4-20ma/ModbusMaster/blob/3a05ff87677a9bdd8e027d6906dc05ca15ca8ade/src/util/crc16.h#L71

/** @ingroup util_crc16
    Processor-independent CRC-16 calculation.
    Polynomial: x^16 + x^15 + x^2 + 1 (0xA001)<br>
    Initial value: 0xFFFF
    This CRC is normally used in disk-drive controllers.
    @param uint16_t crc (0x0000..0xFFFF)
    @param uint8_t a (0x00..0xFF)
    @return calculated CRC (0x0000..0xFFFF)
*/

static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
  int i;

  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }

  return crc;
}

#ifdef DEBUG_CONFIG
// #include "fdrs_checkConfig.h"
#endif

SystemPacket theCmd;
DataReading theData[256];
uint8_t ln;
bool newData;
uint8_t gatewayAddress[] = {MAC_PREFIX, GTWY_MAC};
const uint16_t espnow_size = 250 / sizeof(DataReading);
crcResult crcReturned = CRC_NULL;

uint32_t gtwy_timeout = 0;
uint8_t incMAC[6];
DataReading fdrsData[espnow_size];
DataReading incData[espnow_size];

uint8_t data_count = 0;

uint32_t last_refresh;
void (*callback_ptr)(DataReading);
uint16_t subscription_list[256] = {};
bool active_subs[256] = {};
time_t netTimeOffset=UINT32_MAX;          // Offset network time by the 1/2 the amount of time a ping takes, if value not set then use max value

#include "fdrs_oled.h"
#include "fdrs_debug.h"
#include "fdrs_node_espnow.h"
#include "fdrs_node_time.h"
#include "fdrs_node_lora.h"

void beginFDRS()
{
#ifdef FDRS_DEBUG
  Serial.begin(115200);
  // // find out the reset reason
  // esp_reset_reason_t resetReason;
  // resetReason = esp_reset_reason();
#endif
#ifdef USE_OLED
  init_oled();
  DBG("Display initialized!");
  DBG("Hello, World!");
#endif
  DBG("FDRS User Node initializing...");
  DBG(" Reading ID " + String(READING_ID));
  DBG(" Gateway: " + String(GTWY_MAC, HEX));
#ifdef POWER_CTRL
  DBG("Powering up the sensor array!");
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
  delay(50);
#endif
  // Init ESP-NOW for either ESP8266 or ESP32

#ifdef USE_ESPNOW
  DBG("Initializing ESP-NOW!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
#if defined(ESP8266)
#ifdef USE_LR
  DBG(" LR mode is only available on ESP32. ESP-NOW will begin in normal mode.");
#endif
  if (esp_now_init() != 0)
  {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Register peers
  esp_now_add_peer(gatewayAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)
#ifdef USE_LR
  DBG(" ESP-NOW LR mode is active!");
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
#endif
  if (esp_now_init() != ESP_OK)
  {
    DBG("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBG("Failed to add peer bcast");
    return;
  }
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBG("Failed to add peer");
    return;
  }
#endif
  DBG(" ESP-NOW Initialized.");
#endif // USE_ESPNOW
  begin_lora();
#ifdef DEBUG_CONFIG
  // if (resetReason != ESP_RST_DEEPSLEEP) {
  // checkConfig();
  // }
#endif // DEBUG_CONFIG
}

void handleIncoming()
{
  if (newData)
  {

    newData = false;
    for (int i = 0; i < ln; i++)
    { // Cycle through array of incoming DataReadings for any we are subbed to
      for (int j = 0; j < 255; j++)
      { // Cycle through subscriptions for active entries
        if (active_subs[j])
        {

          if (theData[i].id == subscription_list[j])
          {

            (*callback_ptr)(theData[i]);
          }
        }
      }
    }
  }
}

bool sendFDRS()
{
  if(data_count == 0) {
    return false;
  }
  DBG("Sending FDRS Packet!");
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *)&fdrsData, data_count * sizeof(DataReading));
  esp_now_ack_flag = CRC_NULL;
  while (esp_now_ack_flag == CRC_NULL)
  {
    delay(0);
  }
  if (esp_now_ack_flag == CRC_OK)
  {
    data_count = 0;
    return true;
  }
  else
  {
    data_count = 0;
    return false;
  }
#endif
#ifdef USE_LORA
  crcReturned = transmitLoRa(&gtwyAddress, fdrsData, data_count);
  // DBG(" LoRa sent.");
#ifdef LORA_ACK
  if(crcReturned == CRC_OK) {
  data_count = 0;
    return true;
  }
#endif
#ifndef LORA_ACK
  if(crcReturned == CRC_OK || crcReturned == CRC_NULL) {
    data_count = 0;
  return true;
}
#endif
  else {
    data_count = 0;
    return false;
  }
#endif
}

void loadFDRS(float d, uint8_t t)
{
  DBG("Id: " + String(READING_ID) + " - Type: " + String(t) + " - Data loaded: " + String(d));
  if (data_count > espnow_size)
    sendFDRS();
  DataReading dr;
  dr.id = READING_ID;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}
void loadFDRS(float d, uint8_t t, uint16_t id)
{
  DBG("Id: " + String(id) + " - Type: " + String(t) + " - Data loaded: " + String(d));
  if (data_count > espnow_size)
    sendFDRS();
  DataReading dr;
  dr.id = id;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}
void sleepFDRS(int sleep_time)
{
#ifdef DEEP_SLEEP
  DBG(" Deep sleeping.");
#ifdef ESP32
  esp_sleep_enable_timer_wakeup(sleep_time * 1000000);
  esp_deep_sleep_start();
#endif
#ifdef ESP8266
  ESP.deepSleep(sleep_time * 1000000);
#endif
#endif
  DBG(" Delaying.");
  delay(sleep_time * 1000);
}

void loopFDRS()
{
  updateTime();
  handleLoRa();
  handleIncoming();
  // // TO-DO:
  // if (is_added)
  // {
  //   if ((millis() - last_refresh) >= gtwy_timeout)
  //   {
  //     last_refresh = millis();
  //   }
  // }
}

bool subscribeFDRS(uint16_t sub_id)
{
  for (int i = 0; i < 255; i++)
  {
    if ((subscription_list[i] == sub_id) && (active_subs[i]))
    {
      DBG("You're already subscribed to ID " + String(sub_id));
      return true;
    }
  }
  for (int i = 0; i < 255; i++)
  {
    if (!active_subs[i])
    {
      DBG("Subscribing to DataReading ID " + String(sub_id));
      subscription_list[i] = sub_id;
      active_subs[i] = true;
      return true;
    }
  }
  DBG("No subscription could be established!");
  return false;
}
bool unsubscribeFDRS(uint16_t sub_id)
{
  for (int i = 0; i < 255; i++)
  {
    if ((subscription_list[i] == sub_id) && (active_subs[i]))
    {
      DBG("Removing subscription to ID " + String(sub_id));
      active_subs[i] = false;
      return true;
    }
  }
  DBG("No subscription to remove");
  return false;
}

bool addFDRS(void (*new_cb_ptr)(DataReading))
{
  callback_ptr = new_cb_ptr;
#ifdef USE_ESPNOW
  SystemPacket sys_packet = {.cmd = cmd_add, .param = 0};
  esp_now_send(gatewayAddress, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  DBG("ESP-NOW peer registration request submitted to " + String(gatewayAddress[5]));
  uint32_t add_start = millis();
  is_added = false;
  while ((millis() - add_start) <= 1000) // 1000ms timeout
  {
    yield();
    if (is_added)
    {
      DBG("Registration accepted. Timeout: " + String(gtwy_timeout));
      last_refresh = millis();
      return true;
    }
  }
  DBG("No gateways accepted the request");
  return false;
#endif // USE_ESPNOW
  return true;
}

bool addFDRS(int timeout, void (*new_cb_ptr)(DataReading))
{
  callback_ptr = new_cb_ptr;
#ifdef USE_ESPNOW
  SystemPacket sys_packet = {.cmd = cmd_add, .param = 0};
  esp_now_send(gatewayAddress, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  DBG("ESP-NOW peer registration request submitted to " + String(gatewayAddress[5]));
  uint32_t add_start = millis();
  is_added = false;
  while ((millis() - add_start) <= timeout)
  {
    yield();
    if (is_added)
    {
      DBG("Registration accepted. Timeout: " + String(gtwy_timeout));
      last_refresh = millis();
      return true;
    }
  }
  DBG("No gateways accepted the request");
  return false;
#endif // USE_ESPNOW
  return true;
}

uint32_t pingFDRS(uint32_t timeout)
{
  uint32_t pingResponseMs = UINT32_MAX;
#ifdef USE_ESPNOW
  pingResponseMs = pingFDRSEspNow(gatewayAddress, timeout);
#endif
#ifdef USE_LORA
  pingResponseMs = pingFDRSLoRa(&gtwyAddress, timeout);
#endif
  if(pingResponseMs != UINT32_MAX) {
    netTimeOffset = (pingResponseMs/2)/1000;
    adjTimeforNetDelay(netTimeOffset);
  }
return pingResponseMs;
}
