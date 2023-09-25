#include <WiFiUdp.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <WiFi.h>
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

// select Host IP Address
#if defined(HOST_IPADDRESS)
#define FDRS_HOST_IPADDRESS HOST_IPADDRESS
#elif defined(GLOBAL_HOST_IPADDRESS)
#define FDRS_HOST_IPADDRESS GLOBAL_HOST_IPADDRESS
#else
#endif // HOST_IPADDRESS

// select Gateway IP Address
#if defined(GW_IPADDRESS)
#define FDRS_GW_IPADDRESS GW_IPADDRESS
#elif defined(GLOBAL_GW_IPADDRESS)
#define FDRS_GW_IPADDRESS GLOBAL_GW_IPADDRESS
#else
#endif // GW_IPADDRESS

// select Subnet Address
#if defined(SUBNET_ADDRESS)
#define FDRS_SUBNET_ADDRESS SUBNET_ADDRESS
#elif defined(GLOBAL_SUBNET_ADDRESS)
#define FDRS_SUBNET_ADDRESS GLOBAL_SUBNET_ADDRESS
#else
#endif // SUBNET_ADDRESS

// select DNS1 IP Address configuration
#if defined(DNS1_IPADDRESS)
#define FDRS_DNS1_IPADDRESS DNS1_IPADDRESS
#elif defined(GLOBAL_DNS1_IPADDRESS)
#define FDRS_DNS1_IPADDRESS GLOBAL_DNS1_IPADDRESS
#else
// ASSERT("NO DNS1 IP Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_gateway_config.h");
#endif // DNS1_IPADDRESS

// select DNS2 IP Address configuration
#if defined(DNS2_IPADDRESS)
#define FDRS_DNS2_IPADDRESS DNS2_IPADDRESS
#elif defined(GLOBAL_DNS2_IPADDRESS)
#define FDRS_DNS2_IPADDRESS GLOBAL_DNS2_IPADDRESS
#else
#endif // DNS2_IPADDRESS

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
#ifdef USE_STATIC_IPADDRESS
  uint8_t hostIpAddress[4], gatewayAddress[4], subnetAddress[4], dns2Address[4]; 
#endif
uint8_t dns1Address[4];

// Convert IP Addresses from strings to byte arrays of 4 bytes
void stringToByteArray(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

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
#ifdef USE_STATIC_IPADDRESS
  // Convert from String to byte array
  stringToByteArray(FDRS_HOST_IPADDRESS, '.', hostIpAddress, 4, 10);
  stringToByteArray(FDRS_GW_IPADDRESS, '.', gatewayAddress, 4, 10);
  stringToByteArray(FDRS_SUBNET_ADDRESS, '.', subnetAddress, 4, 10);
  stringToByteArray(FDRS_DNS1_IPADDRESS, '.', dns1Address, 4, 10);
  stringToByteArray(FDRS_DNS2_IPADDRESS, '.', dns2Address, 4, 10);
  WiFi.config(hostIpAddress, gatewayAddress, subnetAddress, dns1Address, dns2Address);
#endif
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    DBG("Connecting to WiFi...");
    DBG(FDRS_WIFI_SSID);
    delay(500);
  }
#endif // USE_ETHERNET
}