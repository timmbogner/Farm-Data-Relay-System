#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif

FDRSPeer peer_list[16];
const uint8_t espnow_size = 250 / sizeof(DataReading);

#ifdef ESP32
esp_now_peer_info_t peerInfo;
#endif

uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const uint8_t mac_prefix[] = {MAC_PREFIX};
uint8_t selfAddress[] = {MAC_PREFIX, UNIT_MAC};
uint8_t incMAC[6];

uint8_t ESPNOW1[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_1};
uint8_t ESPNOW2[] = {MAC_PREFIX, ESPNOW_NEIGHBOR_2};

#ifdef USE_ESPNOW
DataReading ESPNOW1buffer[256];
uint8_t lenESPNOW1 = 0;
uint32_t timeESPNOW1 = 0;
DataReading ESPNOW2buffer[256];
uint8_t lenESPNOW2 = 0;
uint32_t timeESPNOW2 = 0;
DataReading ESPNOWGbuffer[256];
uint8_t lenESPNOWG = 0;
uint32_t timeESPNOWG = 0;
#endif // USE_ESPNOW

#ifdef USE_ESPNOW
// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
}
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
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
#endif // USE_ESPNOW

void begin_espnow()
{
#ifdef USE_ESPNOW
  DBG("Initializing ESP-NOW!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#if defined(ESP8266)
  wifi_set_macaddr(STATION_IF, selfAddress);
  if (esp_now_init() != 0)
  {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

#elif defined(ESP32)
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
  // Register first peer

  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBG("Failed to add peer bcast");
    return;
  }
  // #ifdef ESPNOW1_PEER
  //   memcpy(peerInfo.peer_addr, ESPNOW1, 6);
  //   if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  //     DBG("Failed to add peer 1");
  //     return;
  //   }
  // #endif
  // #ifdef ESPNOW2_PEER
  //   memcpy(peerInfo.peer_addr, ESPNOW2, 6);
  //   if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  //     DBG("Failed to add peer 2");
  //     return;
  //   }
  // #endif
#endif // ESP8266
  DBG(" ESP-NOW Initialized.");
#endif // USE_ESPNOW
}

void add_espnow_peer()
{
  DBG("Device requesting peer registration");
  int peer_num = getFDRSPeer(&incMAC[0]);
  if (peer_num == -1) // if the device isn't registered
  {
    DBG("Device not yet registered, adding to internal peer list");
    int open_peer = find_espnow_peer();                // find open spot in peer_list
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
}
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
void sendESPNOWpeers()
{
#ifdef USE_ESPNOW
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
  esp_now_send(0, (uint8_t *)&thePacket, j * sizeof(DataReading));
#endif // USE_ESPNOW
}

void sendESPNOW(uint8_t address)
{
#ifdef USE_ESPNOW
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

#endif // USE_ESPNOW
}

void bufferESPNOW(uint8_t interface)
{
#ifdef USE_ESPNOW
  DBG("Buffering ESP-NOW.");

  switch (interface)
  {
  case 0:
    for (int i = 0; i < ln; i++)
    {
      ESPNOWGbuffer[lenESPNOWG + i] = theData[i];
    }
    lenESPNOWG += ln;
    break;
  case 1:
    for (int i = 0; i < ln; i++)
    {
      ESPNOW1buffer[lenESPNOW1 + i] = theData[i];
    }
    lenESPNOW1 += ln;
    break;
  case 2:
    for (int i = 0; i < ln; i++)
    {
      ESPNOW2buffer[lenESPNOW2 + i] = theData[i];
    }
    lenESPNOW2 += ln;
    break;
  }

#endif // USE_ESPNOW
}

void releaseESPNOW(uint8_t interface)
{
#ifdef USE_ESPNOW
  DBG("Releasing ESP-NOW.");
  switch (interface)
  {
  case 0:
  {
    DataReading thePacket[espnow_size];
    int j = 0;
    for (int i = 0; i < lenESPNOWG; i++)
    {
      if (j > espnow_size)
      {
        j = 0;
        esp_now_send(broadcast_mac, (uint8_t *)&thePacket, sizeof(thePacket));
      }
      thePacket[j] = ESPNOWGbuffer[i];
      j++;
    }
    esp_now_send(broadcast_mac, (uint8_t *)&thePacket, j * sizeof(DataReading));
    lenESPNOWG = 0;
    break;
  }
  case 1:
  {
    DataReading thePacket[espnow_size];
    int j = 0;
    for (int i = 0; i < lenESPNOW1; i++)
    {
      if (j > espnow_size)
      {
        j = 0;
        esp_now_send(ESPNOW1, (uint8_t *)&thePacket, sizeof(thePacket));
      }
      thePacket[j] = ESPNOW1buffer[i];
      j++;
    }
    esp_now_send(ESPNOW1, (uint8_t *)&thePacket, j * sizeof(DataReading));
    lenESPNOW1 = 0;
    break;
  }
  case 2:
  {
    DataReading thePacket[espnow_size];
    int j = 0;
    for (int i = 0; i < lenESPNOW2; i++)
    {
      if (j > espnow_size)
      {
        j = 0;
        esp_now_send(ESPNOW2, (uint8_t *)&thePacket, sizeof(thePacket));
      }
      thePacket[j] = ESPNOW2buffer[i];
      j++;
    }
    esp_now_send(ESPNOW2, (uint8_t *)&thePacket, j * sizeof(DataReading));
    lenESPNOW2 = 0;
    break;
  }
  }

#endif // USE_ESPNOW
}
