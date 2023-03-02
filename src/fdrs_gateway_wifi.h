#ifdef USE_ETHERNET
#ifndef USE_WIFI
#define USE_WIFI
#endif
#endif // USE_ETHERNET

#ifdef USE_WIFI
#include <WiFiUdp.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#ifdef USE_ETHERNET
#include <ETH.h>
#endif

// select WiFi SSID configuration
#if defined(WIFI_SSID)
#define FDRS_WIFI_SSID WIFI_SSID
#elif defined(GLOBAL_WIFI_SSID)
#define FDRS_WIFI_SSID GLOBAL_WIFI_SSID
#else
// ASSERT("NO WiFi SSID defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // WIFI_SSID

// select WiFi password
#if defined(WIFI_PASS)
#define FDRS_WIFI_PASS WIFI_PASS
#elif defined(GLOBAL_WIFI_PASS)
#define FDRS_WIFI_PASS GLOBAL_WIFI_PASS
#else
// ASSERT("NO WiFi password defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // WIFI_PASS

// select DNS IP Address configuration
#if defined(DNS_IPADDRESS)
#define FDRS_DNS_IPADDRESS DNS_IPADDRESS
#elif defined(GLOBAL_DNS_IPADDRESS)
#define FDRS_DNS_IPADDRESS GLOBAL_DNS_IPADDRESS
#else
// ASSERT("NO DNS IP Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // DNS_IPADDRESS

#ifdef USE_ETHERNET
static bool eth_connected = false;
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

#endif // USE_ETHERNET
const char *ssid = FDRS_WIFI_SSID;
const char *password = FDRS_WIFI_PASS;
IPAddress dnsAddress;

void begin_wifi()
{
  delay(10);
#ifdef USE_ETHERNET
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
    while (!eth_connected)
  {
    DBG("Connecting ethernet...");
    delay(500);
  }
#else
  WiFi.dnsIP(dnsAddress.fromString(FDRS_DNS_IPADDRESS));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    DBG("Connecting to WiFi...");
    DBG(FDRS_WIFI_SSID);
    delay(500);
  }
#endif // USE_ETHERNET
}
#endif // USE_WIFI