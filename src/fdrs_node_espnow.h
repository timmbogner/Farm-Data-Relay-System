#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <espnow.h>
#elif defined(ESP32)
    #include <esp_now.h>
    #include <WiFi.h>
    #include <esp_wifi.h>
#endif

uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
crcResult esp_now_ack_flag;
bool is_added = false;
bool pingFlag = false;

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
    if (len < sizeof(DataReading))
    {
        SystemPacket command;
        memcpy(&command, incomingData, sizeof(command));
        switch (command.cmd)
        {
        case cmd_ping:
            pingFlag = true;
            break;
        case cmd_add:
            is_added = true;
            gtwy_timeout = command.param;
            break;
        }
    }
    else
    {
        memcpy(&theData, incomingData, len);
        ln = len / sizeof(DataReading);
        newData = true;
    }
}

// FDRS node pings gateway and listens for a defined amount of time for a reply
// Blocking function for timeout amount of time (up to timeout time waiting for reply)(IE no callback)
// Returns the amount of time in ms that the ping takes or predefined value if ping fails within timeout
uint32_t pingFDRSEspNow(uint8_t *address, uint32_t timeout) {
    SystemPacket sys_packet = {.cmd = cmd_ping, .param = 0};
    
    esp_now_send(address, (uint8_t *)&sys_packet, sizeof(SystemPacket));
    DBG(" ESP-NOW ping sent.");
    uint32_t ping_start = millis();
    pingFlag = false;
    while ((millis() - ping_start) <= timeout)
    {
        yield(); // do I need to yield or does it automatically?
        if (pingFlag)
        {
            DBG("ESP-NOW Ping Reply in " + String(millis() - ping_start) + "ms from " + String(address[0], HEX) + ":" + String(address[1], HEX) + ":" + String(address[2], HEX) + ":" + String(address[3], HEX) + ":" + String(address[4], HEX) + ":" + String(address[5], HEX));
            return (millis() - ping_start);
        }
    }
    DBG("No ESP-NOW ping returned within " + String(timeout) + "ms.");
    return UINT32_MAX;
}