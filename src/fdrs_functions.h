//  FARM DATA RELAY SYSTEM
//
//  GATEWAY 2.000 Functions
//  This is the 'meat and potatoes' of FDRS, and should not be fooled with unless improving/adding features.
//  Developed by Timm Bogner (timmbogner@gmail.com)

#ifndef __FDRS_FUNCTIONS_H__
#define __FDRS_FUNCTIONS_H__


enum {
  event_clear,
  event_espnowg,
  event_espnow1,
  event_espnow2,
  event_serial,
  event_mqtt,
  event_lorag,
  event_lora1,
  event_lora2
};

enum crcResult{
  CRC_NULL,
  CRC_OK,
  CRC_BAD,
} returnCRC = CRC_NULL;

enum {
  cmd_clear,
  cmd_ping,
  cmd_add,
  cmd_ack
};

#ifdef FDRS_DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#if defined (ESP32)
#define UART_IF Serial1
#else
#define UART_IF Serial
#endif

// enable to get detailed info from where single configuration macros have been taken
#define DEBUG_NODE_CONFIG

#ifdef USE_WIFI

// select WiFi SSID configuration
#if defined(WIFI_SSID)
#define FDRS_WIFI_SSID WIFI_SSID
#elif defined (GLOBAL_SSID)
#define FDRS_WIFI_SSID GLOBAL_SSID
#else 
// ASSERT("NO WiFi SSID defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //WIFI_SSID

// select WiFi password 
#if defined(WIFI_PASS)
#define FDRS_WIFI_PASS WIFI_PASS
#elif defined (GLOBAL_PASS)
#define FDRS_WIFI_PASS GLOBAL_PASS
#else 
// ASSERT("NO WiFi password defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //WIFI_PASS

// select MQTT server address
#if defined(MQTT_ADDR)
#define FDRS_MQTT_ADDR MQTT_ADDR
#elif defined (GLOBAL_MQTT_ADDR)
#define FDRS_MQTT_ADDR GLOBAL_MQTT_ADDR
#else 
// ASSERT("NO MQTT address defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //MQTT_ADDR

// select MQTT server port
#if defined(MQTT_PORT)
#define FDRS_MQTT_PORT MQTT_PORT
#elif defined (GLOBAL_MQTT_PORT)
#define FDRS_MQTT_PORT GLOBAL_MQTT_PORT
#else 
#define FDRS_MQTT_PORT 1883
#endif //MQTT_PORT

// select MQTT user name
#if defined(MQTT_USER)
#define FDRS_MQTT_USER MQTT_USER
#elif defined (GLOBAL_MQTT_USER)
#define FDRS_MQTT_USER GLOBAL_MQTT_USER
#else 
// ASSERT("NO MQTT user defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //MQTT_USER

// select MQTT user password
#if defined(MQTT_PASS)
#define FDRS_MQTT_PASS MQTT_PASS
#elif defined (GLOBAL_MQTT_PASS)
#define FDRS_MQTT_PASS GLOBAL_MQTT_PASS
#else 
// ASSERT("NO MQTT password defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //MQTT_PASS

#if defined (MQTT_AUTH) || defined (GLOBAL_MQTT_AUTH)
#define FDRS_MQTT_AUTH
#endif //MQTT_AUTH

#endif //USE_WIFI

#ifdef USE_LORA

// select LoRa band configuration
#if defined(LORA_BAND)
#define FDRS_BAND LORA_BAND
#elif defined (GLOBAL_LORA_BAND)
#define FDRS_BAND GLOBAL_LORA_BAND
#else 
// ASSERT("NO LORA-BAND defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //LORA_BAND

// select LoRa SF configuration
#if defined(LORA_SF)
#define FDRS_SF LORA_SF
#elif defined (GLOBAL_LORA_SF)
#define FDRS_SF GLOBAL_LORA_SF
#else 
// ASSERT("NO LORA-SF defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //LORA_SF

// select LoRa TXPWR configuration
#if defined(LORA_TXPWR)
#define FDRS_TXPWR LORA_TXPWR
#elif defined (GLOBAL_LORA_TXPWR)
#define FDRS_TXPWR GLOBAL_LORA_TXPWR
#else 
// ASSERT("NO LORA-TXPWR defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
#endif //LORA_TXPWR

#endif //USE_LORA

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

#ifdef DEBUG_NODE_CONFIG
#include "fdrs_checkConfig.h"
#endif
typedef struct FDRSPeer {
  uint8_t mac[6];
  uint32_t last_seen = 0;

} FDRSPeer;
typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

typedef struct __attribute__((packed)) SystemPacket {
  uint8_t cmd;
  uint32_t param;
} SystemPacket;

FDRSPeer peer_list[16];
const uint8_t espnow_size = 250 / sizeof(DataReading);
const uint8_t lora_size   = 256 / sizeof(DataReading);
const uint8_t mac_prefix[] = {MAC_PREFIX};

#ifdef ESP32
esp_now_peer_info_t peerInfo;
#endif

uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t selfAddress[] =   {MAC_PREFIX, UNIT_MAC};
uint8_t incMAC[6];

#ifdef ESPNOW1_PEER
uint8_t ESPNOW1[] =       {MAC_PREFIX, ESPNOW1_PEER};
#else
uint8_t ESPNOW1[] =       {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
#ifdef ESPNOW2_PEER
uint8_t ESPNOW2[] =       {MAC_PREFIX, ESPNOW2_PEER};
#else
uint8_t ESPNOW2[] =       {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif

#ifdef USE_LORA
uint16_t LoRa1 =         ((mac_prefix[4] << 8) | LORA1_PEER);  // Use 2 bytes for LoRa addressing instead of previous 3 bytes
uint16_t LoRa2 =         ((mac_prefix[4] << 8) | LORA2_PEER);
uint16_t loraGwAddress = ((selfAddress[4] << 8) | selfAddress[5]); // last 2 bytes of gateway address
uint16_t loraBroadcast = 0xFFFF;
unsigned long receivedLoRaMsg = 0;  // Number of total LoRa packets destined for us and of valid size
unsigned long ackOkLoRaMsg = 0;     // Number of total LoRa packets with valid CRC
#endif

#if defined (USE_SD_LOG) || defined (USE_FS_LOG)
char logBuffer[512];
uint16_t logBufferPos = 0; // datatype depends on size of sdBuffer
uint32_t timeLOGBUF = 0;
time_t last_mqtt_success = 0;
time_t last_log_write = 0;
#endif

SystemPacket theCmd;
DataReading theData[256];
uint8_t ln;
uint8_t newData = event_clear;
uint8_t newCmd = cmd_clear;
bool is_ping = false;

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
#endif //USE_ESPNOW

DataReading SERIALbuffer[256];
uint8_t lenSERIAL = 0;
uint32_t timeSERIAL = 0;
DataReading MQTTbuffer[256];
uint8_t lenMQTT = 0;
uint32_t timeMQTT = 0;

#ifdef USE_LORA
DataReading LORAGbuffer[256];
uint8_t lenLORAG = 0;
uint32_t timeLORAG = 0;
DataReading LORA1buffer[256];
uint8_t lenLORA1 = 0;
uint32_t timeLORA1 = 0;
DataReading LORA2buffer[256];
uint8_t lenLORA2 = 0;
uint32_t timeLORA2 = 0;
#endif //USE_LORA

#ifdef USE_LED
CRGB leds[NUM_LEDS];
#endif //USE_LED

#ifdef USE_WIFI
WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = FDRS_WIFI_SSID;
const char* password = FDRS_WIFI_PASS;
const char* mqtt_server = FDRS_MQTT_ADDR;
const int mqtt_port = FDRS_MQTT_PORT;

#ifdef FDRS_MQTT_AUTH
const char* mqtt_user = FDRS_MQTT_USER;
const char* mqtt_pass = FDRS_MQTT_PASS;
#else
const char* mqtt_user = NULL;
const char* mqtt_pass = NULL;
#endif //FDRS_MQTT_AUTH

#endif //USE_WIFI


// Function prototypes
void transmitLoRa(uint16_t*, DataReading*, uint8_t);
void transmitLoRa(uint16_t*, SystemPacket*, uint8_t);
static uint16_t crc16_update(uint16_t, uint8_t);


// CRC16 from https://github.com/4-20ma/ModbusMaster/blob/3a05ff87677a9bdd8e027d6906dc05ca15ca8ade/src/util/crc16.h#L71

/** @ingroup util_crc16
    Processor-independent CRC-16 calculation.
    Polynomial: x^16 + x^15 + x^2 + 1 (0xA001)<br>
    Initial value: 0xFFFF
    This CRC is normally used in disk-drive controllers.
    @param uint16_t crc (0x0000..0xFFFF)
    @param uint8_t a (0x00..0xFF)
    @return calculated CRC (0x0000..0xFFFF)
*/

static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
  int i;

  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }

  return crc;
}



#include <fdrs_lora.h>
#include <fdrs_espnow.h>

void getSerial() {
  String incomingString =  UART_IF.readStringUntil('\n');
  DynamicJsonDocument doc(24576);
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    //    DBG("json parse err");
    //    DBG(incomingString);
    return;
  } else {
    int s = doc.size();
    //UART_IF.println(s);
    for (int i = 0; i < s; i++) {
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = event_serial;
    DBG("Incoming Serial.");

  }
}

#if defined (USE_SD_LOG) || defined (USE_FS_LOG)
void releaseLogBuffer()
{
#ifdef USE_SD_LOG
  DBG("Releasing Log buffer to SD");
  File logfile = SD.open(SD_FILENAME, FILE_WRITE);
  if((logfile.size()/1024.0) < SD_MAX_FILESIZE){
    logfile.print(logBuffer);
  }
  logfile.close();
#endif
#ifdef USE_FS_LOG
  DBG("Releasing Log buffer to internal flash.");
  File logfile = LittleFS.open(FS_FILENAME, "a");
  if((logfile.size()/1024.0) < FS_MAX_FILESIZE){
    logfile.print(logBuffer);
  }
  logfile.close();
#endif
  memset(&(logBuffer[0]), 0, sizeof(logBuffer) / sizeof(char));
  logBufferPos = 0;
}
#endif // USE_XX_LOG

uint16_t stringCrc(const char input[]){
  uint16_t calcCRC = 0x0000;

  for(unsigned int i = 0; i < strlen(input); i++) {
    calcCRC = crc16_update(calcCRC,input[i]);
  }
  return calcCRC;
}

void sendLog()
{
#if defined (USE_SD_LOG) || defined (USE_FS_LOG)
  DBG("Logging to buffer");
  for (int i = 0; i < ln; i++)
  {
    StaticJsonDocument<96> doc;
    JsonObject doc_0 = doc.createNestedObject();
    doc_0["id"] = theData[i].id;
    doc_0["type"] = theData[i].t;
    doc_0["data"] = theData[i].d;
    doc_0["time"] = time(nullptr);
    String outgoingString;
    serializeJson(doc, outgoingString);
    outgoingString = outgoingString + " " + stringCrc(outgoingString.c_str()) + "\r\n";
    if (logBufferPos+outgoingString.length() >= (sizeof(logBuffer)/sizeof(char))) // if buffer would overflow, release first
    {
      releaseLogBuffer();
    }
    memcpy(&logBuffer[logBufferPos], outgoingString.c_str(), outgoingString.length()); //append line to buffer
    logBufferPos+=outgoingString.length();
  }
  time(&last_log_write);
  #endif //USE_xx_LOG
}

void reconnect(short int attempts, bool silent) {
#ifdef USE_WIFI

  if (!silent) DBG("Connecting MQTT...");

  for (short int i = 1; i <= attempts; i++) {
    // Attempt to connect
    if (client.connect("FDRS_GATEWAY", mqtt_user, mqtt_pass)) {
      // Subscribe
      client.subscribe(TOPIC_COMMAND);
      if (!silent) DBG(" MQTT Connected");
      return;
    } else {
      if (!silent) {
        char msg[23];
        sprintf(msg, " Attempt %d/%d", i, attempts);
        DBG(msg);
      }
      if ((attempts = !1)) {
        delay(3000);
      }
    }
  }

  if (!silent) DBG(" Connecting MQTT failed.");
#endif //USE_WIFI
}

void reconnect(int attempts) {
  reconnect(attempts, false);
}

void mqtt_callback(char* topic, byte * message, unsigned int length) {
  String incomingString;
  DBG(topic);
  for (unsigned int i = 0; i < length; i++) {
    incomingString += (char)message[i];
  }
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    DBG("json parse err");
    DBG(incomingString);
    return;
  } else {
    int s = doc.size();
    //UART_IF.println(s);
    for (int i = 0; i < s; i++) {
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = event_mqtt;
    DBG("Incoming MQTT.");

  }
}

void resendLog(){
  #ifdef USE_SD_LOG
  DBG("Resending logged values from SD card.");
  File logfile = SD.open(SD_FILENAME, FILE_READ);
  while(1){
    String line = logfile.readStringUntil('\n');
    if (line.length() > 0){  // if line contains something
      if (!client.publish(TOPIC_DATA_BACKLOG, line.c_str())) {
        break;
      }else{
        time(&last_mqtt_success);
      }
    }else{
      logfile.close();
      SD.remove(SD_FILENAME); // if all values are sent
      break;
    }
  }
  DBG(" Done");
  #endif
  #ifdef USE_FS_LOG
  DBG("Resending logged values from internal flash.");
  File logfile = LittleFS.open(FS_FILENAME, "r");
  while(1){
    String line = logfile.readStringUntil('\n');
    if (line.length() > 0){  // if line contains something
      uint16_t readCrc;
      char data[line.length()];
      sscanf(line.c_str(),"%s %hd",data,&readCrc);
      if(stringCrc(data)!=readCrc){continue;} // if CRCs don't match, skip the line
      if (!client.publish(TOPIC_DATA_BACKLOG, line.c_str())) {
        break;
      }else{
        time(&last_mqtt_success);
      }
    }else{
      logfile.close();
      LittleFS.remove(FS_FILENAME); // if all values are sent
      break;
    }
  }
  DBG(" Done");
  #endif

}

void mqtt_publish(const char* payload) {
#ifdef USE_WIFI
  if (!client.publish(TOPIC_DATA, payload)) {
    DBG(" Error on sending MQTT");
    sendLog();
  }else{
    #if defined (USE_SD_LOG) || defined (USE_FS_LOG)
      if (last_log_write >= last_mqtt_success){
        releaseLogBuffer();
        resendLog();
      }
      time(&last_mqtt_success);
    #endif
  }
#endif //USE_WIFI
}

void sendSerial() {
  DBG("Sending Serial.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < ln; i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
  }
  serializeJson(doc, UART_IF);
  UART_IF.println();

#ifndef ESP8266
  serializeJson(doc, Serial);
  Serial.println();
#endif

}
void sendMQTT() {
#ifdef USE_WIFI
  DBG("Sending MQTT.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < ln; i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
    doc[i]["time"] = time(nullptr);
  }
  String outgoingString;
  serializeJson(doc, outgoingString);
  mqtt_publish((char*) outgoingString.c_str());
#endif //USE_WIFI
}

void bufferSerial() {
  DBG("Buffering Serial.");
  for (int i = 0; i < ln; i++) {
    SERIALbuffer[lenSERIAL + i] = theData[i];
  }
  lenSERIAL += ln;
  //UART_IF.println("SENDSERIAL:" + String(lenSERIAL) + " ");
}

void bufferMQTT() {
  DBG("Buffering MQTT.");
  for (int i = 0; i < ln; i++) {
    MQTTbuffer[lenMQTT + i] = theData[i];
  }
  lenMQTT += ln;
}

//void bufferLoRa() {
//  for (int i = 0; i < ln; i++) {
//    LORAbuffer[lenLORA + i] = theData[i];
//  }
//  lenLORA += ln;
//}


void releaseMQTT() {
#ifdef USE_WIFI
  DBG("Releasing MQTT.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < lenMQTT; i++) {
    doc[i]["id"]   = MQTTbuffer[i].id;
    doc[i]["type"] = MQTTbuffer[i].t;
    doc[i]["data"] = MQTTbuffer[i].d;
  }
  String outgoingString;
  serializeJson(doc, outgoingString);
  mqtt_publish((char*) outgoingString.c_str());
  lenMQTT = 0;
#endif //USE_WIFI
}

void begin_SD() {
#ifdef USE_SD_LOG
  DBG("Initializing SD card...");
#ifdef ESP32
  SPI.begin(SCK, MISO, MOSI);
#endif
  if (!SD.begin(SD_SS)) {
    DBG(" Initialization failed!");
    while (1);
  } else {
    DBG(" SD initialized.");
  }
#endif //USE_SD_LOG
}

void begin_FS() {
#ifdef USE_FS_LOG
  DBG("Initializing LittleFS...");

  if (!LittleFS.begin())
  {
    DBG(" initialization failed");
    while (1);
  }
  else
  {
    DBG(" LittleFS initialized");
  }
#endif // USE_FS_LOG
}

int getFDRSPeer(uint8_t *mac) {  // Returns the index of the array element that contains the provided MAC address
  DBG("Getting peer #");

  for (int i = 0; i < 16; i++) {
    if (memcmp(mac, &peer_list[i].mac, 6) == 0) {
      DBG("Peer is entry #" + String(i));
      return i;
    }
  }

  //DBG("Couldn't find peer");
  return -1;
}
int findOpenPeer() {    // Returns an expired entry in peer_list, -1 if full.
  //uint8_t zero_addr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  for (int i = 0; i < 16; i++) {
   if (peer_list[i].last_seen == 0){
      DBG("Using peer entry " + String(i));
      return i;
   }
  }
    for (int i = 0; i < 16; i++) {
    if ((millis() - peer_list[i].last_seen) >= PEER_TIMEOUT){
      DBG("Recycling peer entry " + String(i));
            esp_now_del_peer(peer_list[i].mac);

      return i;
   }
  }
  DBG("No open peers");
  return -1;
}
int checkPeerExpired() {  // Checks whether any entries in the peer_list have expired. Not currently used.
  for (int i = 0; i < 16; i++) {
    if ((millis() - peer_list[i].last_seen) >= PEER_TIMEOUT) {
      esp_now_del_peer(incMAC);
    }
    return -1;
  }
}


void handleCommands() {
  switch (theCmd.cmd) {
    case cmd_ping:
      DBG("Ping back to sender");
      SystemPacket sys_packet;
      sys_packet.cmd = cmd_ping;
      if (!esp_now_is_peer_exist(incMAC)) {
#ifdef ESP8266
        esp_now_add_peer(incMAC, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#endif
#if defined(ESP32)
        esp_now_peer_info_t peerInfo;
        peerInfo.ifidx = WIFI_IF_STA;
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        memcpy(peerInfo.peer_addr, incMAC, 6);
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
          DBG("Failed to add peer");
          return;
        }
#endif
        esp_now_send(incMAC, (uint8_t *) &sys_packet, sizeof(SystemPacket));
        esp_now_del_peer(incMAC);
      } else {
        esp_now_send(incMAC, (uint8_t *) &sys_packet, sizeof(SystemPacket));
      }
      break;

    case cmd_add:
      DBG("Device requesting peer registration");
      int peer_num = getFDRSPeer(&incMAC[0]);
      if (peer_num == -1) {  //if the device isn't registered
        DBG("Device not yet registered, adding to internal peer list");
        int open_peer = findOpenPeer();   // find open spot in peer_list
        memcpy(&peer_list[open_peer].mac, &incMAC, 6); //save MAC to open spot
        peer_list[open_peer].last_seen = millis();
#if defined(ESP32)
        esp_now_peer_info_t peerInfo;
        peerInfo.ifidx = WIFI_IF_STA;
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        memcpy(peerInfo.peer_addr, incMAC, 6);
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
          DBG("Failed to add peer");
          return;
        }
#endif
#if defined(ESP8266)
        esp_now_add_peer(incMAC, ESP_NOW_ROLE_COMBO, 0, NULL, 0);

#endif
        SystemPacket sys_packet = { .cmd = cmd_add, .param = PEER_TIMEOUT };
        esp_now_send(incMAC, (uint8_t *) &sys_packet, sizeof(SystemPacket));

      } else {
        DBG("Refreshing existing peer registration");
        peer_list[peer_num].last_seen = millis();

        SystemPacket sys_packet = { .cmd = cmd_add, .param = PEER_TIMEOUT };
        esp_now_send(incMAC, (uint8_t *) &sys_packet, sizeof(SystemPacket));


      }
      break;

  }

  theCmd.cmd = cmd_clear;
  theCmd.param = 0;


}

#endif //__FDRS_FUNCTIONS_H__
