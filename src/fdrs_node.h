//  FARM DATA RELAY SYSTEM
//
//  "fdrs_node.h"
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//
#include <fdrs_datatypes.h>
#include <fdrs_globals.h>
#define FDRS_NODE

bool is_controller = false;
DataReading theData[256];
uint8_t ln;
uint8_t newData = event_clear;
uint8_t gatewayAddress[] = {MAC_PREFIX, GTWY_MAC};
const uint16_t espnow_size = (250 - 15) / sizeof(DataReading); // 250 bytes minus 15 bytes for addressing and other data
crcResult crcReturned = CRC_NULL;

uint8_t incMAC[6];
DataReading fdrsData[espnow_size];
DataReading incData[espnow_size];
TimeSource timeSource;

uint8_t data_count = 0;

void (*callback_ptr)(DataReading);
uint16_t subscription_list[256] = {};
bool active_subs[256] = {};
unsigned long lastTimePrint = 0;

void sendTimeSerial();


#ifdef USE_I2C
  #include <Wire.h>
#endif
#ifdef USE_OLED
  #include "fdrs_oled.h"
#endif
#include "fdrs_debug.h"
#include "fdrs_time.h"
#ifdef DEBUG_CONFIG
// #include "fdrs_checkConfig.h"
#endif
#ifdef USE_ESPNOW
  #include "fdrs_node_espnow.h"
#endif
#ifdef USE_LORA
  #include "fdrs_lora.h"
#endif

void beginFDRS()
{
  Serial.begin(115200);
  // // find out the reset reason
  // esp_reset_reason_t resetReason;
  // resetReason = esp_reset_reason();
#ifdef USE_I2C
  Wire.begin(I2C_SDA, I2C_SCL);
#endif
#ifdef USE_OLED
  init_oled();
  DBG("Display initialized!");
  DBG("Hello, World!");
#endif
  DBG("");
  DBG("Initializing FDRS Node!");
  DBG("Reading ID " + String(READING_ID));
  DBG("Gateway: " + String(GTWY_MAC, HEX));
  DBG("Debugging verbosity level: " + String(DBG_LEVEL));
#ifdef USE_ESPNOW
  DBG1("ESP-NOW is enabled.");
#endif
#ifdef USE_LORA
  DBG1("LoRa is enabled.");
#endif
#ifdef DEEP_SLEEP
  DBG1("Deep sleep is enabled.");
#endif
#ifdef POWER_CTRL
  DBG1("Power control is enabled on pin " + String(POWER_CTRL));
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
    DBG(" Failed to add peer bcast");
    return;
  }
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBG(" Failed to add peer");
    return;
  }
#endif
  DBG(" ESP-NOW Initialized.");
#endif // USE_ESPNOW
#ifdef USE_LORA
  begin_lora();
#endif
#ifdef DEBUG_CONFIG
  // if (resetReason != ESP_RST_DEEPSLEEP) {
  // checkConfig();
  // }
#endif // DEBUG_CONFIG
}

void handleIncoming()
{
  if (newData != event_clear)
  {

    newData = event_clear;
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

// Sends FDRS data synchronously
// Returns true if success and false if not successful
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
    yield();
    delay(0);
  }
  if (esp_now_ack_flag == CRC_OK)
  {
    data_count = 0;
    DBG1("FDRS Packet sent successfully!");
    return true;
  }
  else
  {
    data_count = 0;
    DBG1("FDRS Packet send failed!");
    return false;
  }
#endif // USE_ESPNOW
#ifdef USE_LORA
  crcResult crcReturned = CRC_NULL;
  crcReturned = transmitLoRaSync(&gtwyAddress, fdrsData, data_count);
  DBG(" LoRa sent.");
  if(crcReturned == CRC_OK) {
    data_count = 0;
    DBG1("FDRS Packet sent successfully!");
    return true;
  }
  else if(ack == false && crcReturned == CRC_NULL) {
    data_count = 0;
    DBG1("FDRS Packet sent successfully!");
    return true;
  }
  else {
    data_count = 0;
    DBG1("FDRS Packet send failed!");
    return false;
  }
#endif // USE_LORA
}

// Sends FDRS data asynchronously - no spinning loop, more processor friendly, but does not return as much information
// Returns true if data is queued up, false if there is no data to be sent
bool sendFDRSAsync()
{
  if(data_count == 0) {
    return false;
  }

  DBG("Queueing FDRS Packet!");
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *)&fdrsData, data_count * sizeof(DataReading));
  data_count = 0;
  return true;
#endif // USE_ESPNOW
#ifdef USE_LORA
  transmitLoRaAsync(&gtwyAddress, fdrsData, data_count);
  data_count = 0;
  return true;
#endif // USE_LORA
}

void loadFDRS(float d, uint8_t t)
{
  DBG("Id: " + String(READING_ID) + " - Type: " + String(t) + " - Data loaded: " + String(d));
  if (data_count >= espnow_size)
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
  if (data_count >= espnow_size)
    sendFDRS();
  DataReading dr;
  dr.id = id;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}

void sleepFDRS(uint32_t sleep_time)
{
  // make sure all Async operations are completed
#ifdef USE_LORA
  unsigned long timeout = millis() + 1000;
  while(millis() < timeout && !isLoRaAsyncComplete()) {
    handleLoRa();
    yield();
  }
#endif // USE_LORA
#ifdef DEEP_SLEEP
  DBG(" Deep sleeping.");
#ifdef ESP32
  esp_sleep_enable_timer_wakeup(sleep_time * 1000000ULL);
  esp_deep_sleep_start();
#endif
#ifdef ESP8266
  ESP.deepSleep(sleep_time * 1000000);
#endif
#endif // DEEP_SLEEP
  DBG(" Delaying.");
    delay(sleep_time * 1000);
}



void loopFDRS()
{
  handleTime();
#ifdef USE_LORA
  handleLoRa();
#endif
if (is_controller){
  handleIncoming();
#ifdef USE_ESPNOW
    if (TDIFF(last_refresh,gtwy_timeout))
    {
      refresh_registration();
      last_refresh = millis();
    }
#endif 
  }
  // Output time to display if time is valid
  if(TDIFFMIN(lastTimePrint,FDRS_TIME_PRINTTIME)) {
    lastTimePrint = millis();
    printTime();
  }
}

bool addFDRS(void (*new_cb_ptr)(DataReading))
{
  is_controller = true;  
  callback_ptr = new_cb_ptr;
#ifdef USE_ESPNOW
  SystemPacket sys_packet = {.cmd = cmd_add, .param = 0};
  DBG("ESP-NOW peer registration request submitted to 0x" + String(gatewayAddress[5],HEX));
  uint32_t add_start = millis();
  is_added = false;
  esp_now_send(gatewayAddress, (uint8_t *)&sys_packet, sizeof(SystemPacket));
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
  is_controller = true;  
  callback_ptr = new_cb_ptr;
#ifdef USE_ESPNOW
  SystemPacket sys_packet = {.cmd = cmd_add, .param = 0};
  DBG("ESP-NOW peer registration request submitted to 0x" + String(gatewayAddress[5],HEX));
  uint32_t add_start = millis();
  is_added = false;
  esp_now_send(gatewayAddress, (uint8_t *)&sys_packet, sizeof(SystemPacket));
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



int pingFDRS(uint32_t timeout)
{
  int pingResult = -1;
#ifdef USE_ESPNOW
  pingResult = pingFDRSEspNow(gatewayAddress, timeout);
#endif
#ifdef USE_LORA
  pingResult = pingRequestLoRa(gtwyAddress, timeout);
#endif
  return pingResult;
}

bool reqTimeFDRS() {
#ifdef USE_ESPNOW
  return reqTimeEspNow();
#endif
#ifdef USE_LORA
  return reqTimeLoRa();
#endif
}

// Skeleton Functions related to function calls to files that are not included
#ifndef USE_LORA
  void sendTimeLoRa() {}
  bool reqTimeLoRa() { return false; }
#endif
#ifndef USE_ESPNOW
  esp_err_t sendTimeESPNow() { return ESP_OK; }                  // fdrs_gateway_time.h
#endif
void sendTimeSerial() { }
