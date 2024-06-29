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

// uint8_t ESPNOW1[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_1};
// uint8_t ESPNOW2[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_2};
// uint8_t ESPNOW3[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_3};
// uint8_t ESPNOW4[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_4};


uint8_t ESPNOW_Nbr[4][6] = {{MAC_PREFIX, ESPNOW_NEIGHBOR_0},
                          {MAC_PREFIX, ESPNOW_NEIGHBOR_1},
                          {MAC_PREFIX, ESPNOW_NEIGHBOR_2},
                          {MAC_PREFIX, ESPNOW_NEIGHBOR_3}};
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
  memcpy(&incMAC, mac, sizeof(incMAC));
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  esp_now_sent_flag = true;
}
void OnDataRecv(const esp_now_recv_info *pkt_info, const uint8_t *incomingData, int len)
{
  memcpy(&incMAC, pkt_info->src_addr, sizeof(incMAC));
  #endif
  if (len < sizeof(DataReading))
  {
    DBG1("Incoming ESP-NOW System Packet from 0x" + String(incMAC[5], HEX));
    memcpy(&theCmd, incomingData, sizeof(theCmd));
    // processing is handled in the handlecommands() function in gateway.h - do not process here
    return;
  }
  else {
    memcpy(&theData, incomingData, sizeof(theData));
    DBG("Incoming ESP-NOW DataReading from 0x" + String(incMAC[5], HEX));
    ln = len / sizeof(DataReading);
    if (memcmp(&incMAC, &ESPNOW_Nbr[0], 6) == 0)
    {
      newData = event_espnow0;
      return;
    }
    if (memcmp(&incMAC, &ESPNOW_Nbr[1], 6) == 0)
    {
      newData = event_espnow1;
      return;
    }
    if (memcmp(&incMAC, &ESPNOW_Nbr[2], 6) == 0)
    {
      newData = event_espnow2;
      return;
    }
    if (memcmp(&incMAC, &ESPNOW_Nbr[3], 6) == 0)
    {
      newData = event_espnow3;
      return;
    }
    newData = event_espnowg;
  }
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
      DBG1("Using peer entry " + String(i));
      return i;
    }
  }
  for (int i = 0; i < 16; i++)
  {
    if (TDIFF(peer_list[i].last_seen,PEER_TIMEOUT))
    {
      DBG1("Recycling peer entry " + String(i));
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
  DBG2("Getting peer #");

  for (int i = 0; i < 16; i++)
  {
    if (memcmp(mac, &peer_list[i].mac, 6) == 0)
    {
      DBG1("Peer is entry #" + String(i));
      return i;
    }
  }

  DBG1("Couldn't find peer");
  return -1;
}

void add_espnow_peer()
{
  DBG1("Device requesting peer registration");
  int peer_num = getFDRSPeer(&incMAC[0]);
  if (peer_num == -1) // if the device isn't registered
  {
    int open_peer = find_espnow_peer();            // find open spot in peer_list
    DBG("Registering new peer. Slot: " + String(open_peer));
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
    DBG1("Refreshing existing peer registration");
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
  DBG("Sending ESP-NOW Ping Reply");
  SystemPacket sys_packet;
  sys_packet = { .cmd = cmd_ping, .param = ping_reply };
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

void sendESPNowNbr(uint8_t neighbor)
{
  DBG("Sending data to ESP-NOW Neighbor #" + String(neighbor));
#if defined(ESP32)
  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, ESPNOW_Nbr[neighbor], 6);
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
      esp_now_send(ESPNOW_Nbr[neighbor], (uint8_t *)&thePacket, sizeof(thePacket));
    }
    thePacket[j] = theData[i];
    j++;
  }
  esp_now_send(ESPNOW_Nbr[neighbor], (uint8_t *)&thePacket, j * sizeof(DataReading));
  esp_now_del_peer(ESPNOW_Nbr[neighbor]);
}

void sendESPNowPeers()
{
  DBG("Sending data to ESP-NOW peers.");
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

// Lower level function meant to be called by other functions 
// Sends SystemPacket via ESP-NOW
esp_err_t sendESPNow(uint8_t *dest, SystemPacket *data) {
  esp_err_t sendResult;
  if (dest != nullptr && !esp_now_is_peer_exist(dest))
  {
#ifdef ESP8266
    sendResult = esp_now_add_peer(dest, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
    if (sendResult != ESP_OK)
    {
      DBG("Failed to add peer");
      return sendResult;
    }
#endif
#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, dest, 6);
    sendResult = esp_now_add_peer(&peerInfo);
    if (sendResult != ESP_OK)
    {
      DBG("Failed to add peer");
      return sendResult;
    }
#endif
    sendResult = esp_now_send(dest, (uint8_t *)data, sizeof(SystemPacket));
    esp_now_del_peer(dest);
  }
  else
  {
    sendResult = esp_now_send(dest, (uint8_t *)data, sizeof(SystemPacket));
  }
  return sendResult;
}

// Lower level function meant to be called by other functions 
// Sends DataReading via ESP-NOW
esp_err_t sendESPNow(uint8_t *dest, DataReading *data) {
  esp_err_t sendResult;
  bool tempPeerFlag = false;


  if (dest != nullptr && !esp_now_is_peer_exist(dest))
  {
    tempPeerFlag = true;
#ifdef ESP8266
    sendResult = esp_now_add_peer(dest, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
    if (sendResult != ESP_OK)
    {
      DBG("Failed to add peer");
      return sendResult;
    }
  }
#endif
#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, dest, 6);
    sendResult = esp_now_add_peer(&peerInfo);
    if (sendResult != ESP_OK)
    {
      DBG("Failed to add peer");
      return sendResult;
    }
  }
#endif // ESP32
    for(int i = 0; i < ln; ) {
      if(ln > espnow_size) {
        sendResult = esp_now_send(dest, (uint8_t *)&data[i], espnow_size * sizeof(DataReading));
        if(sendResult == ESP_OK) {
          i += espnow_size;
        }
        else {
          // Send failed!
          delay(10);
          return sendResult;
        }
      }
      else {
        sendResult = esp_now_send(dest, (uint8_t *)&data[i], ln * sizeof(DataReading));
        if(sendResult == ESP_OK) {
          ln = 0;
        }
        else {
          // Send Failed!
          delay(10);
          return sendResult;
        }
      }
    } 
    if(tempPeerFlag) {
      esp_now_del_peer(dest);
    }
    return sendResult;
}



void sendESPNow(uint8_t address)
{
  DBG("Sending ESP-NOW DR.");
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
  // Process time if there is no time source set yet or if LoRa is the time source or if we are already the time source
  if(timeSource.tmNetIf <= TMIF_ESPNOW ) {
    DBG1("Received time via ESP-NOW from 0x" + String(incMAC[5], HEX));
    if(timeSource.tmNetIf < TMIF_ESPNOW) {
      timeSource.tmNetIf = TMIF_ESPNOW;
      timeSource.tmAddress = incMAC[4] << 8 | incMAC[5];
      timeSource.tmSource = TMS_NET;
      DBG1("ESP-NOW time source is 0x" + String(incMAC[5], HEX));
    }
    if(timeSource.tmAddress == incMAC[4] << 8 | incMAC[5]) {
      if(setTime(t)) {
        timeSource.tmLastTimeSet = millis();
      }
    }
  }
  else {
    DBG2("ESP-NOW 0x" + String(incMAC[5], HEX) + " is not time source, discarding request");
  }
  return;
}

// Sends time to both neighbors and all peers
esp_err_t sendTimeESPNow() {
  
  esp_err_t result1 = ESP_OK, result2 = ESP_OK, result3 = ESP_OK;
  SystemPacket sys_packet = { .cmd = cmd_time, .param = now };

for (uint8_t i = 0; i < 4; i++){
  if((timeSource.tmAddress != (ESPNOW_Nbr[i][4] << 8 | ESPNOW_Nbr[i][5])) && ESPNOW_Nbr[i][5] != 0x00) {
    DBG1("Sending time to ESP-NOW Neighbor #" + String(i));
    result1 = sendESPNow(ESPNOW_Nbr[i], &sys_packet);
  }
}

  DBG1("Sending time to ESP-NOW registered peers");
  result3 = sendESPNow(nullptr, &sys_packet);

  if(result1 != ESP_OK || result2 != ESP_OK || result3 != ESP_OK){
    return ESP_FAIL;
  }
  else {
    return ESP_OK;
  }
}

// Send the time to a specific node
esp_err_t sendTimeESPNow(uint8_t *addr) {
  
  esp_err_t result = ESP_FAIL;
  SystemPacket sys_packet = { .cmd = cmd_time, .param = now };
  DBG1("Sending time to ESP-NOW address 0x" + String(addr[5],HEX));
  result = sendESPNow(addr, &sys_packet);

  return result;
}