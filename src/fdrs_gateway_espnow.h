#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif

#define PEER_TIMEOUT 300000
FDRSPeer peer_list[16];
const uint8_t espnow_size = 250 / sizeof(DataReading);

#ifdef ESP32
esp_now_peer_info_t peerInfo;
#endif
bool esp_now_sent_flag;
const uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const uint8_t mac_prefix[] = {MAC_PREFIX};
uint8_t selfAddress[] = {MAC_PREFIX, UNIT_MAC};
uint8_t incMAC[6];

uint8_t ESPNOW1[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_1};
uint8_t ESPNOW2[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_2};
extern time_t now;
// JL - pingFlagEspNow var probably to be removed
bool pingFlagEspNow = false;

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  esp_now_sent_flag = true;
}
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    esp_now_sent_flag = true;
}
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
#endif
  memcpy(&incMAC, mac, sizeof(incMAC));
  if (len < sizeof(DataReading))
  {
    DBG("ESP-NOW System Packet");
    memcpy(&theCmd, incomingData, sizeof(theCmd));
    memcpy(&incMAC, mac, sizeof(incMAC));
    return;
  }
  memcpy(&theData, incomingData, sizeof(theData));
  DBG("Incoming ESP-NOW.");
  ln = len / sizeof(DataReading);
  if (memcmp(&incMAC, &ESPNOW1, 6) == 0)
  {
    newData = event_espnow1;
    return;
  }
  if (memcmp(&incMAC, &ESPNOW2, 6) == 0)
  {
    newData = event_espnow2;
    return;
  }
  newData = event_espnowg;
}

void begin_espnow()
{
  DBG("Initializing ESP-NOW!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#if defined(ESP8266)
#ifdef USE_LR
  DBG(" LR mode is only available on ESP32. ESP-NOW will begin in normal mode.");
#endif
  wifi_set_macaddr(STATION_IF, selfAddress);
  if (esp_now_init() != 0)
  {
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

#elif defined(ESP32)
#ifdef USE_LR
  DBG(" ESP-NOW LR mode is active!");
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
#endif
  esp_wifi_set_mac(WIFI_IF_STA, &selfAddress[0]);
  if (esp_now_init() != ESP_OK)
  {
    DBG("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBG("Failed to add peer bcast");
    return;
  }
#endif // ESP8266
  DBG(" ESP-NOW Initialized.");
}

// Returns an expired entry in peer_list, -1 if full.
int find_espnow_peer()
{
  for (int i = 0; i < 16; i++)
  {
    if (peer_list[i].last_seen == 0)
    {
      // DBG("Using peer entry " + String(i));
      return i;
    }
  }
  for (int i = 0; i < 16; i++)
  {
    if ((millis() - peer_list[i].last_seen) >= PEER_TIMEOUT)
    {
      // DBG("Recycling peer entry " + String(i));
      esp_now_del_peer(peer_list[i].mac);

      return i;
    }
  }
  DBG("No open peers");
  return -1;
}

// Returns the index of the peer list array element that contains the provided MAC address, -1 if not found
int getFDRSPeer(uint8_t *mac)
{
  // DBG("Getting peer #");

  for (int i = 0; i < 16; i++)
  {
    if (memcmp(mac, &peer_list[i].mac, 6) == 0)
    {
      DBG("Peer is entry #" + String(i));
      return i;
    }
  }

  // DBG("Couldn't find peer");
  return -1;
}

void add_espnow_peer()
{
  DBG("Device requesting peer registration");
  int peer_num = getFDRSPeer(&incMAC[0]);
  if (peer_num == -1) // if the device isn't registered
  {
    int open_peer = find_espnow_peer();            // find open spot in peer_list
    DBG("New device will be registered as " + String(open_peer));
    memcpy(&peer_list[open_peer].mac, &incMAC, 6); // save MAC to open spot
    peer_list[open_peer].last_seen = millis();
#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, incMAC, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      DBG("Failed to add peer");
      return;
    }
#endif
#if defined(ESP8266)
    esp_now_add_peer(incMAC, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

#endif
    SystemPacket sys_packet = {.cmd = cmd_add, .param = PEER_TIMEOUT};
    esp_now_send(incMAC, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  }
  else
  {
    DBG("Refreshing existing peer registration");
    peer_list[peer_num].last_seen = millis();

    SystemPacket sys_packet = {.cmd = cmd_add, .param = PEER_TIMEOUT};
    esp_now_send(incMAC, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  }
if(validTimeFlag){
    SystemPacket sys_packet = { .cmd = cmd_time, .param = now };
    esp_now_send(incMAC, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  }
}

// Sends ping reply to sender
void pingback_espnow()
{
  DBG("Ping back to sender");
  SystemPacket sys_packet;
  sys_packet.cmd = cmd_ping;
  if (!esp_now_is_peer_exist(incMAC))
  {
#ifdef ESP8266
    esp_now_add_peer(incMAC, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#endif
#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, incMAC, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      DBG("Failed to add peer");
      return;
    }
#endif
    esp_now_send(incMAC, (uint8_t *)&sys_packet, sizeof(SystemPacket));
    esp_now_del_peer(incMAC);
  }
  else
  {
    esp_now_send(incMAC, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  }
}

void sendESPNowNbr(uint8_t interface)
{
  switch (interface)
  {
    case 1:
    { // These brackets are required!
      DBG("Sending to ESP-NOW Neighbor #1");
#if defined(ESP32)
      esp_now_peer_info_t peerInfo;
      peerInfo.ifidx = WIFI_IF_STA;
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      memcpy(peerInfo.peer_addr, ESPNOW1, 6);
      if (esp_now_add_peer(&peerInfo) != ESP_OK)
      {
        DBG("Failed to add peer");
        return;
      }
#endif // ESP32
      DataReading thePacket[ln];
      int j = 0;

      for (int i = 0; i < ln; i++)
      {
        if (j > espnow_size)
        {
          j = 0;
          esp_now_send(ESPNOW1, (uint8_t *)&thePacket, sizeof(thePacket));
        }
        thePacket[j] = theData[i];
        j++;
      }
      esp_now_send(ESPNOW1, (uint8_t *)&thePacket, j * sizeof(DataReading));
      esp_now_del_peer(ESPNOW1);

      break;
    } // These brackets are required!
    case 2:
    {
      DBG("Sending to ESP-NOW Neighbor #2");
#if defined(ESP32)
      esp_now_peer_info_t peerInfo;
      peerInfo.ifidx = WIFI_IF_STA;
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      memcpy(peerInfo.peer_addr, ESPNOW2, 6);
      if (esp_now_add_peer(&peerInfo) != ESP_OK)
      {
        DBG("Failed to add peer");
        return;
      }
#endif // ESP32
      DataReading thePacket[ln];
      int j = 0;
      for (int i = 0; i < ln; i++)
      {
        if (j > espnow_size)
        {
          j = 0;
          esp_now_send(ESPNOW2, (uint8_t *)&thePacket, sizeof(thePacket));
        }
        thePacket[j] = theData[i];
        j++;
      }
      esp_now_send(ESPNOW2, (uint8_t *)&thePacket, j * sizeof(DataReading));
      esp_now_del_peer(ESPNOW2);

      break;
    }
  }
}

void sendESPNowPeers()
{
  DBG("Sending to ESP-NOW peers.");
  DataReading thePacket[ln];
  int j = 0;
  for (int i = 0; i < ln; i++)
  {
    if (j > espnow_size)
    {
      j = 0;
      esp_now_send(0, (uint8_t *)&thePacket, sizeof(thePacket));
    }
    thePacket[j] = theData[i];
    j++;
  }
  for (int i = 0; i < 16; i++)
  {
    if (peer_list[i].last_seen != 0 && (millis() - peer_list[i].last_seen) < PEER_TIMEOUT)
    {
      //uint32_t clktm = millis();
      esp_now_sent_flag = false;
      esp_now_send(peer_list[i].mac, (uint8_t *)&thePacket, j * sizeof(DataReading));
      while (!esp_now_sent_flag) yield();
      //DBG(millis() - clktm);
    }
  }


}

void sendESPNow(uint8_t address)
{
  DBG("Sending ESP-NOW.");
  uint8_t temp_peer[] = {MAC_PREFIX, address};
#if defined(ESP32)
  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, temp_peer, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBG("Failed to add peer");
    return;
  }
#endif // ESP32

  DataReading thePacket[ln];
  int j = 0;
  for (int i = 0; i < ln; i++)
  {
    if (j > espnow_size)
    {
      j = 0;
      esp_now_send(temp_peer, (uint8_t *)&thePacket, sizeof(thePacket));
    }
    thePacket[j] = theData[i];
    j++;
  }

  esp_now_send(temp_peer, (uint8_t *)&thePacket, j * sizeof(DataReading));
  esp_now_del_peer(temp_peer);
}

void recvTimeEspNow(uint32_t t) {
  // Process time if there is no master set yet or if LoRa is the master or if we are already the time master
  if(timeMaster.tmType == TM_NONE || timeMaster.tmType == TM_LORA || (timeMaster.tmType == TM_ESPNOW && timeMaster.tmAddress == incMAC[4] << 8 | incMAC[5])) {
    DBG("Received time via ESP-NOW from 0x" + String(incMAC[5], HEX));
    if(timeMaster.tmAddress == 0x0000) {
      timeMaster.tmType = TM_ESPNOW;
      timeMaster.tmAddress = incMAC[4] << 8 & incMAC[5];
      DBG("ESP-NOW time master is 0x" + String(incMAC[5], HEX));
    }
    setTime(t);
    timeMaster.tmLastTimeSet = millis();
  }
  else {
    DBG("ESP-NOW 0x" + String(incMAC[5], HEX) + " is not time master, discarding request");
  }
  return;
}

// Sends time to both neighbors and all peers
esp_err_t sendTimeESPNow() {
  
  esp_err_t result1 = ESP_OK, result2 = ESP_OK, result3 = ESP_OK;
  SystemPacket sys_packet = { .cmd = cmd_time, .param = now };

  if((timeMaster.tmAddress != ESPNOW1[4] << 8 | ESPNOW1[5]) && ESPNOW1[5] != 0x00) {
    DBG("Sending time to ESP-NOW Peer 1");
    result1 = sendESPNow(ESPNOW1, &sys_packet);
  }
  if((timeMaster.tmAddress != ESPNOW2[4] << 8 | ESPNOW2[5]) && ESPNOW2[5] != 0x00) {
    DBG("Sending time to ESP-NOW Peer 2");
    result2 = sendESPNow(ESPNOW2, &sys_packet);
  }
  DBG("Sending time to ESP-NOW registered peers");
  result3 = sendESPNow(nullptr, &sys_packet);

  if(result1 != ESP_OK || result2 != ESP_OK || result3 != ESP_OK){
    return ESP_FAIL;
  }
  else {
    return ESP_OK;
  }
}