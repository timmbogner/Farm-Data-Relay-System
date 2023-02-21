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

#ifdef USE_ESPNOW
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
            is_ping = true;
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
#endif // USE_ESPNOW
