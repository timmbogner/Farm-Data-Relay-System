
#ifdef USE_WIFI
#include <WiFiUdp.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#endif
// select WiFi SSID configuration
#if defined(WIFI_SSID)
#define FDRS_WIFI_SSID WIFI_SSID
#elif defined (GLOBAL_WIFI_SSID)
#define FDRS_WIFI_SSID GLOBAL_WIFI_SSID
#else 
// ASSERT("NO WiFi SSID defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif //WIFI_SSID

// select WiFi password 
#if defined(WIFI_PASS)
#define FDRS_WIFI_PASS WIFI_PASS
#elif defined (GLOBAL_WIFI_PASS)
#define FDRS_WIFI_PASS GLOBAL_WIFI_PASS
#else 
// ASSERT("NO WiFi password defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif //WIFI_PASS

const char *ssid = FDRS_WIFI_SSID;
const char *password = FDRS_WIFI_PASS;

void begin_wifi(){
      delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    DBG("Connecting to WiFi...");
    DBG(FDRS_WIFI_SSID);
    delay(500);
  }
}
#endif // USE_WIFI
