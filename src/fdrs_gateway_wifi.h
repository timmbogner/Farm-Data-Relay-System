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

// select DNS IP Address configuration
#if defined(DNS_IPADDRESS)
#define FDRS_DNS_IPADDRESS DNS_IPADDRESS
#elif defined(GLOBAL_DNS_IPADDRESS)
#define FDRS_DNS_IPADDRESS GLOBAL_DNS_IPADDRESS
#else
// ASSERT("NO DNS IP Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // DNS_IPADDRESS

// select NTP Time Server configuration
#if defined(TIME_SERVER)
#define FDRS_TIME_SERVER TIME_SERVER
#else 
#define FDRS_TIME_SERVER GLOBAL_TIME_SERVER
#endif // TIME_SERVER

// select Time, in minutes, between NTP time server queries configuration
#if defined(TIME_FETCHNTP)
#define FDRS_TIME_FETCHNTP TIME_FETCHNTP
#else
#define FDRS_TIME_FETCHNTP GLOBAL_TIME_FETCHNTP
#endif // TIME_FETCHNTP


WiFiUDP FDRSNtp;
unsigned int localPort = 8888;        // local port to listen for UDP packets
const char timeServer[] = FDRS_TIME_SERVER; // NTP server
const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];   //buffer to hold incoming and outgoing packets
uint NTPFetchFail = 0;                // consecutive NTP fetch failures
extern time_t now;

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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    DBG("Connecting to WiFi...");
    DBG(FDRS_WIFI_SSID);
    delay(500);
  }
#endif // USE_ETHERNET
}

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  FDRSNtp.beginPacket(address, 123); // NTP requests are to port 123
  FDRSNtp.write(packetBuffer, NTP_PACKET_SIZE);
  FDRSNtp.endPacket();
}

void fetchNtpTime() {
  //DBG("GetTime Function");
  if(WiFi.status() == WL_CONNECTED) {
    //DBG("Calling .begin function");
    FDRSNtp.begin(localPort);

    sendNTPpacket(timeServer); // send an NTP packet to a time server
    uint i = 0;
    for(i = 0; i < 800; i++) {
      if(FDRSNtp.parsePacket())
        break;
      delay(10);
    }
    if(i < 800) {
      DBG("Took " + String(i * 10) + "ms to get NTP response from " + String(timeServer) + ".");
      NTPFetchFail = 0;
      // We've received a packet, read the data from it
      FDRSNtp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      // the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, extract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      //DBG("Seconds since Jan 1 1900 = " + String(secsSince1900));
      
      // now convert NTP time into everyday time:
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      // now is epoch format - seconds since Jan 1 1970
      now = secsSince1900 - seventyYears;
      setTime(now); // UTC time
    }
    else {
      NTPFetchFail++;
      DBG("Timeout getting a NTP response. " + String(NTPFetchFail) + " consecutive failures.");
      // If unable to Update the time after N tries then set the time to be not valid.
      if(NTPFetchFail > 5) {
        validTimeFlag = false;
        DBG("Time no longer reliable.");
      }
    }
  }
}

void begin_ntp() {
  fetchNtpTime();
  updateTime();
  printTime();
}