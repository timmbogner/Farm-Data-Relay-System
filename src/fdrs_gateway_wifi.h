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

const char *ssid = FDRS_WIFI_SSID;
const char *password = FDRS_WIFI_PASS;
#ifdef USE_STATIC_IPADDRESS
  uint8_t hostIpAddress[4], gatewayAddress[4], subnetAddress[4], dns1Address[4], dns2Address[4]; 
#endif

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
int connectTries = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    connectTries++;
    DBG("Connecting to WiFi SSID: " + String(FDRS_WIFI_SSID) + " try number " + String(connectTries));
    delay(1000);
    WiFi.reconnect();
    if(connectTries >= 15) {
        DBG("Restarting ESP32: WiFi issues\n");
        delay(5000);  
        ESP.restart();
}
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
  if(timeSource.tmSource <= TMS_NTP) {
#ifdef USE_ETHERNET
  if(eth_connected) {
#elif defined(USE_WIFI)
  if(WiFi.status() == WL_CONNECTED) {
#endif
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
      DBG2("Took " + String(i * 10) + "ms to get NTP response from " + String(timeServer) + ".");
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
      if(setTime(now)) {
          timeSource.tmNetIf = TMIF_LOCAL;
          timeSource.tmAddress = 0xFFFF;
          timeSource.tmSource = TMS_NTP;
          timeSource.tmLastTimeSet = millis();
          DBG1("Time source is now local NTP");
        } // UTC time
          }
    else {
      DBG1("Timeout getting a NTP response.");
      }
    }
  }
  return;
}

void begin_ntp() {
  fetchNtpTime();
  handleTime();
  printTime();
}