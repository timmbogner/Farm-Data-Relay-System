#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <espnow.h>
#elif defined(ESP32)
    #include <esp_now.h>
    #include <WiFi.h>
    #include <esp_wifi.h>
#endif

enum PingStatusEspNow {
  psNotStarted,
  psWaiting,
  psCompleted,
};

typedef struct EspNowPing {
  PingStatusEspNow status = psNotStarted;
  unsigned long start;
  uint timeout;
  uint8_t address[6];
  uint32_t response = __UINT32_MAX__;
} EspNowPing;

EspNowPing espNowPing;

uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
crcResult esp_now_ack_flag;
bool is_added = false;
bool pingFlag = false;
uint32_t last_refresh = 0;
uint32_t gtwy_timeout = 300000;

// Request time from gateway - Optionally used in sensors
bool reqTimeEspNow() {
    unsigned long pingStart = millis();
    SystemPacket sys_packet = {.cmd = cmd_time, .param = 0};
    DBG1("Requesting time from gateway 0x" + String(gatewayAddress[5],HEX));
    esp_now_send(gatewayAddress, (uint8_t *)&sys_packet, sizeof(SystemPacket));
    while(timeSource.tmNetIf < TMIF_ESPNOW && (millis() - pingStart < 1000)) {
        // wait for time to be set
        // magic happens here :)
    }
    if(timeSource.tmNetIf == TMIF_ESPNOW) {
        return true;
    }
    else {
        return false;
    }
}

void recvTimeEspNow(uint32_t t) {
  // Process time if there is no master set yet or if LoRa is the master or if we are already the time master
  if(timeSource.tmNetIf < TMIF_ESPNOW || (timeSource.tmNetIf == TMIF_ESPNOW && timeSource.tmAddress == (incMAC[4] << 8 | incMAC[5]))) {
    DBG("Received time via ESP-NOW from 0x" + String(incMAC[5], HEX));
    if(timeSource.tmNetIf < TMIF_ESPNOW) {
        timeSource.tmNetIf = TMIF_ESPNOW;
        timeSource.tmSource = TMS_NET;
        timeSource.tmAddress = incMAC[4] << 8 & incMAC[5];
        DBG("ESP-NOW time source is now 0x" + String(incMAC[5], HEX));
    }
    setTime(t);
    timeSource.tmLastTimeSet = millis();
  }
  else {
    DBG("ESP-NOW 0x" + String(incMAC[5], HEX) + " is not our time source, discarding request");
  }
  return;
}

uint32_t recvPingEspNow(uint8_t *srcAddr) {
    uint32_t response = UINT32_MAX;

    if(memcmp(espNowPing.address, srcAddr, sizeof(espNowPing.address)) == 0) {

        if(TDIFF(espNowPing.start,espNowPing.timeout)) {
            DBG1("No ESP-NOW ping returned within " + String(espNowPing.timeout) + "ms.");
        }
        else {
            espNowPing.response = millis() - espNowPing.start;
            DBG1("ESP-NOW Ping Reply in " + String(espNowPing.response) + "ms from 0x" + String(espNowPing.address[5], HEX));
            response = espNowPing.response;
        }
    }
    else {
        DBG1("ESP-NOW ping reply from unexpected source 0x" + String(*(srcAddr + 5),HEX));
    }

    // reset status
    espNowPing.status = psNotStarted;
    espNowPing.timeout = 0;
    espNowPing.start = 0;
    memcpy(espNowPing.address, broadcast_mac, sizeof(espNowPing.address));
    espNowPing.response = 0;

    return response;
}

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
    if (sendStatus == 0)
    {
        esp_now_ack_flag = CRC_OK;
    }
    else
    {
        esp_now_ack_flag = CRC_BAD;
    }
}
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        esp_now_ack_flag = CRC_OK;
    }
    else
    {
        esp_now_ack_flag = CRC_BAD;
    }
}
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
#endif
memcpy(&incMAC, mac, sizeof(incMAC));
    if (len == sizeof(SystemPacket))
    {
        SystemPacket command;
        memcpy(&command, incomingData, sizeof(command));
        DBG2("Incoming ESP-NOW System Packet from 0x" + String(incMAC[5], HEX));
        switch (command.cmd)
        {
        case cmd_ping:
            if(command.param == ping_reply) {
                recvPingEspNow(incMAC);
            }
            break;
        case cmd_add:
            is_added = true;
            gtwy_timeout = command.param;
            break;
        case cmd_time:
            if(command.param > MIN_TS) {
                recvTimeEspNow(command.param);
            }
            break;
        }
    }
    else if((len == sizeof(DataReading)))
    {
        memcpy(&theData, incomingData, len);
        ln = len / sizeof(DataReading);
        DBG2("Incoming ESP-NOW Data Reading from 0x" + String(incMAC[5], HEX));
        newData = event_espnowg;
        // Processing done by handleIncoming() in fdrs_node.h
    }
    else {
        DBG2("Incoming ESP-NOW Data from 0x" + String(incMAC[5], HEX) + " of unexpected size " + String(len));
    }
}

// FDRS node pings gateway and listens for a defined amount of time for a reply
// Asynchonous call so does not wait for a reply so we do not know how long the ping takes
// ESP-NOW is on the order of 10 milliseconds so happens very quickly.  Not sure Async is warranted.
void pingFDRSEspNow(uint8_t *dstaddr, uint32_t timeout) {
    SystemPacket sys_packet = {.cmd = cmd_ping, .param = 0};
    
    // ping function called again after previous ping not received for some reason
    if(espNowPing.status == psWaiting) {
        espNowPing.status = psNotStarted;
        espNowPing.timeout = 0;
        espNowPing.start = 0;
        espNowPing.response = 0;
        memcpy(espNowPing.address, broadcast_mac, sizeof(espNowPing.address));
    }

    espNowPing.status = psWaiting;
    espNowPing.timeout = timeout;
    espNowPing.start = millis();
    espNowPing.response = 0;
    memcpy(espNowPing.address, dstaddr, sizeof(espNowPing.address));
    DBG1("ESP-NOW ping sent to 0x" + String(*(espNowPing.address + 5),HEX));
    esp_now_send(dstaddr, (uint8_t *)&sys_packet, sizeof(SystemPacket));
    
}

bool refresh_registration()
{
#ifdef USE_ESPNOW
  SystemPacket sys_packet = {.cmd = cmd_add, .param = 0};
  esp_now_send(gatewayAddress, (uint8_t *)&sys_packet, sizeof(SystemPacket));
  DBG("Refreshing registration to 0x" + String(gatewayAddress[5],HEX));
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

esp_err_t sendTimeESPNow() {
    return ESP_OK;
}