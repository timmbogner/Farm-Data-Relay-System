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

    memcpy(&incMAC, mac, sizeof(incMAC));

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
        memcpy(&incData, incomingData, len);
        int pkt_readings = len / sizeof(DataReading);
        for (int i = 0; i <= pkt_readings; i++)
        { // Cycle through array of incoming DataReadings for any addressed to this device
            for (int j = 0; j < 255; j++)
            { // Cycle through subscriptions for active entries
                if (active_subs[j])
                {
                    if (incData[i].id == subscription_list[j])
                    {
                        (*callback_ptr)(incData[i]);
                    }
                }
            }
        }
    }
}
#endif // USE_ESPNOW

bool seekFDRS(int timeout)
{
    SystemPacket sys_packet = {.cmd = cmd_ping, .param = 0};
#ifdef USE_ESPNOW
    esp_now_send(broadcast_mac, (uint8_t *)&sys_packet, sizeof(SystemPacket));
    DBG("Seeking nearby gateways");
    uint32_t ping_start = millis();
    is_ping = false;
    while ((millis() - ping_start) <= timeout)
    {
        yield(); // do I need to yield or does it automatically?
        if (is_ping)
        {
            DBG("Responded:" + String(incMAC[5]));
            return true;
        }
    }
    return false;
#endif
}
