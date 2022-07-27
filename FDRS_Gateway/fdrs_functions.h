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

#endif //USE_LORA

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

#ifdef DEBUG_NODE_CONFIG
#include "fdrs_checkConfig.h"
#endif

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

typedef struct __attribute__((packed)) SystemPacket {
  uint8_t cmd;
  uint32_t param;
} SystemPacket;

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
//uint16_t LoRaAddress = 0x4200;
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


#ifdef USE_ESPNOW
// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
}
void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  if (len < sizeof(DataReading)) {
    DBG("ESP-NOW System Packet");
    memcpy(&theCmd, incomingData, sizeof(theCmd));
    memcpy(&incMAC, mac, sizeof(incMAC));
    return;
  }
  memcpy(&theData, incomingData, sizeof(theData));
  memcpy(&incMAC, mac, sizeof(incMAC));
  DBG("Incoming ESP-NOW.");
  ln = len / sizeof(DataReading);
  if (memcmp(&incMAC, &ESPNOW1, 6) == 0) {
    newData = event_espnow1;
    return;
  }
  if (memcmp(&incMAC, &ESPNOW2, 6) == 0) {
    newData = event_espnow2;
    return;
  }
  newData = event_espnowg;
}
#endif //USE_ESPNOW

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
#endif

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
    outgoingString+="\r\n";
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
      if (!client.publish(TOPIC_DATA, line.c_str())) {
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
      if (!client.publish(TOPIC_DATA, line.c_str())) {
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

void printLoraPacket(uint8_t* p,int size) {
  printf("Printing packet of size %d.",size);
  for(int i = 0; i < size; i++ ) {
    if(i % 2 == 0) printf("\n%02d: ", i);
    printf("%02X ", p[i]);
  }
  printf("\n");
}

void getLoRa() {
#ifdef USE_LORA
  int packetSize = LoRa.parsePacket();
  if((packetSize - 6) % sizeof(DataReading) == 0 && packetSize > 0) {  // packet size should be 6 bytes plus multiple of size of DataReading
    uint8_t packet[packetSize];
    uint16_t packetCRC = 0x0000; // CRC Extracted from received LoRa packet
    uint16_t calcCRC = 0x0000; // CRC calculated from received LoRa packet
    uint16_t sourceMAC = 0x0000;
    uint16_t destMAC = 0x0000;
  
    LoRa.readBytes((uint8_t *)&packet, packetSize);
    ln = (packetSize - 6) / sizeof(DataReading);
    
    destMAC = (packet[0] << 8) | packet[1];
    sourceMAC = (packet[2] << 8) | packet[3];
    packetCRC = ((packet[packetSize - 2] << 8) | packet[packetSize - 1]);
    //DBG("Packet Address: 0x" + String(packet[0],16) + String(packet[1],16) + " Self Address: 0x" + String(selfAddress[4],16) + String(selfAddress[5],16));
    if (destMAC == (selfAddress[4] << 8 | selfAddress[5])) {   //Check if addressed to this device (2 bytes, bytes 1 and 2)
      //printLoraPacket(packet,sizeof(packet));
      memcpy(&theData, &packet[4], packetSize - 6);   //Split off data portion of packet (N - 6 bytes (6 bytes for headers and CRC))
      if(receivedLoRaMsg != 0){  // Avoid divide by 0
        DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(LoRa.packetRssi()) + "dBi, SNR: " + String(LoRa.packetSnr()) + "dB, PacketCRC: 0x" + String(packetCRC,16) + ", Total LoRa received: " + String(receivedLoRaMsg) + ", CRC Ok Pct " + String((float)ackOkLoRaMsg/receivedLoRaMsg*100) + "%");
      }
      else {
        DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(LoRa.packetRssi()) + "dBi, SNR: " + String(LoRa.packetSnr()) + "dB, PacketCRC: 0x" + String(packetCRC,16) + ", Total LoRa received: " + String(receivedLoRaMsg));
      }
      receivedLoRaMsg++;
      // Evaluate CRC
      for(int i = 0; i < (packetSize - 2); i++) { // Last 2 bytes of packet are the CRC so do not include them in calculation
        //printf("CRC: %02X : %d\n",calcCRC, i);
        calcCRC = crc16_update(calcCRC, packet[i]);
      }
      if(calcCRC == packetCRC) {
        SystemPacket ACK = { .cmd = cmd_ack, .param = CRC_OK };
        DBG("CRC Match, sending ACK packet to sensor 0x" + String(sourceMAC,16) + "(hex)");
        transmitLoRa(&sourceMAC, &ACK, 1);  // Send ACK back to source
        ackOkLoRaMsg++;
      }
      else if(packetCRC == crc16_update(calcCRC,0xA1)) { // Sender does not want ACK and CRC is valid
        DBG("Sensor address 0x" + String(sourceMAC,16) + "(hex) does not want ACK");
        ackOkLoRaMsg++;
      }
      else {
        SystemPacket NAK = { .cmd = cmd_ack, .param = CRC_BAD };
        // Send NAK packet to sensor
        DBG("CRC Mismatch! Packet CRC is 0x" + String(packetCRC,16) + ", Calculated CRC is 0x" + String(calcCRC,16) + " Sending NAK packet to sensor 0x" + String(sourceMAC,16) + "(hex)");
        transmitLoRa(&sourceMAC, &NAK, 1); // CRC did not match so send NAK to source
        newData = event_clear;  // do not process data as data may be corrupt
        return;  // Exit function and do not update newData to send invalid data further on
      }
    
      if (memcmp(&sourceMAC, &LoRa1, 2) == 0) {      //Check if it is from a registered sender
        newData = event_lora1;
        return;
      }
      if (memcmp(&sourceMAC, &LoRa2, 2) == 0) {
        newData = event_lora2;
        return;
      }
      newData = event_lorag;
    }
    else {
      DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received from address 0x" + String(sourceMAC,16) + " destined for node address 0x" + String(destMAC,16));
    }
  }
  else {
    if(packetSize != 0) {
      DBG("Incoming LoRa packet of " + String(packetSize) + "bytes not processed.");
    }
  }
#endif
}

#ifdef USE_LORA
void transmitLoRa(uint16_t* destMac, DataReading * packet, uint8_t len) {
  uint16_t calcCRC = 0x0000;

  uint8_t pkt[6 + (len * sizeof(DataReading))];
  
  pkt[0] = (*destMac >> 8);       // high byte of destination MAC
  pkt[1] = (*destMac & 0x00FF);   // low byte of destination MAC
  pkt[2] = selfAddress[4];    // high byte of source MAC (ourselves)
  pkt[3] = selfAddress[5];    // low byte of source MAC
  memcpy(&pkt[4], packet, len * sizeof(DataReading));   // copy data portion of packet
  for(int i = 0; i < (sizeof(pkt) - 2); i++) {  // Last 2 bytes are CRC so do not include them in the calculation itself
    //printf("CRC: %02X : %d\n",calcCRC, i);
    calcCRC = crc16_update(calcCRC, pkt[i]);
  }
  pkt[(len * sizeof(DataReading) + 4)] = (calcCRC >> 8); // Append calculated CRC to the last 2 bytes of the packet
  pkt[(len * sizeof(DataReading) + 5)] = (calcCRC & 0x00FF);
  DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC,16) + " to LoRa MAC 0x" + String(*destMac,16));
  //printLoraPacket(pkt,sizeof(pkt));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
  }

#endif //USE_LORA

void sendESPNOW(uint8_t address) {
#ifdef USE_ESPNOW
  DBG("Sending ESP-NOW.");
  uint8_t temp_peer[] = {MAC_PREFIX, address};
#if defined(ESP32)
  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, temp_peer, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer");
    return;
  }
#endif

  DataReading thePacket[ln];
  int j = 0;
  for (int i = 0; i < ln; i++) {
    if ( j > espnow_size) {
      j = 0;
      esp_now_send(temp_peer, (uint8_t *) &thePacket, sizeof(thePacket));
    }
    thePacket[j] = theData[i];
    j++;
  }


  esp_now_send(temp_peer, (uint8_t *) &thePacket, j * sizeof(DataReading));
  esp_now_del_peer(temp_peer);

#endif
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

void bufferESPNOW(uint8_t interface) {
#ifdef USE_ESPNOW
  DBG("Buffering ESP-NOW.");

  switch (interface) {
    case 0:
      for (int i = 0; i < ln; i++) {
        ESPNOWGbuffer[lenESPNOWG + i] = theData[i];
      }
      lenESPNOWG +=  ln;
      break;
    case 1:
      for (int i = 0; i < ln; i++) {
        ESPNOW1buffer[lenESPNOW1 + i] = theData[i];
      }
      lenESPNOW1 +=  ln;
      break;
    case 2:
      for (int i = 0; i < ln; i++) {
        ESPNOW2buffer[lenESPNOW2 + i] = theData[i];
      }
      lenESPNOW2 +=  ln;
      break;
  }
#endif //USE_ESPNOW
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

void bufferLoRa(uint8_t interface) {
#ifdef USE_LORA
  DBG("Buffering LoRa.");
  switch (interface) {
    case 0:
      for (int i = 0; i < ln; i++) {
        LORAGbuffer[lenLORAG + i] = theData[i];
      }
      lenLORAG += ln;
      break;
    case 1:
      for (int i = 0; i < ln; i++) {
        LORA1buffer[lenLORA1 + i] = theData[i];
      }
      lenLORA1 += ln;
      break;
    case 2:
      for (int i = 0; i < ln; i++) {
        LORA2buffer[lenLORA2 + i] = theData[i];
      }
      lenLORA2 += ln;
      break;
  }
#endif //USE_LORA
}

void releaseESPNOW(uint8_t interface) {
#ifdef USE_ESPNOW
  DBG("Releasing ESP-NOW.");
  switch (interface) {
    case 0:
      {
        DataReading thePacket[espnow_size];
        int j = 0;
        for (int i = 0; i < lenESPNOWG; i++) {
          if ( j > espnow_size) {
            j = 0;
            esp_now_send(broadcast_mac, (uint8_t *) &thePacket, sizeof(thePacket));
          }
          thePacket[j] = ESPNOWGbuffer[i];
          j++;
        }
        esp_now_send(broadcast_mac, (uint8_t *) &thePacket, j * sizeof(DataReading));
        lenESPNOWG = 0;
        break;
      }
    case 1:
      {
        DataReading thePacket[espnow_size];
        int j = 0;
        for (int i = 0; i < lenESPNOW1; i++) {
          if ( j > espnow_size) {
            j = 0;
            esp_now_send(ESPNOW1, (uint8_t *) &thePacket, sizeof(thePacket));
          }
          thePacket[j] = ESPNOW1buffer[i];
          j++;
        }
        esp_now_send(ESPNOW1, (uint8_t *) &thePacket, j * sizeof(DataReading));
        lenESPNOW1 = 0;
        break;
      }
    case 2:
      {
        DataReading thePacket[espnow_size];
        int j = 0;
        for (int i = 0; i < lenESPNOW2; i++) {
          if ( j > espnow_size) {
            j = 0;
            esp_now_send(ESPNOW2, (uint8_t *) &thePacket, sizeof(thePacket));
          }
          thePacket[j] = ESPNOW2buffer[i];
          j++;
        }
        esp_now_send(ESPNOW2, (uint8_t *) &thePacket, j * sizeof(DataReading));
        lenESPNOW2 = 0;
        break;
      }
  }
#endif //USE_ESPNOW
}


void releaseLoRa(uint8_t interface) {
#ifdef USE_LORA
  DBG("Releasing LoRa.");

  switch (interface) {
    case 0:
      {
        DataReading thePacket[lora_size];
        int j = 0;
        for (int i = 0; i < lenLORAG; i++) {
          if ( j > lora_size) {
            j = 0;
            transmitLoRa(&loraBroadcast, thePacket, j);
          }
          thePacket[j] = LORAGbuffer[i];
          j++;
        }
        transmitLoRa(&loraBroadcast, thePacket, j);
        lenLORAG = 0;

        break;
      }
    case 1:
      {
        DataReading thePacket[lora_size];
        int j = 0;
        for (int i = 0; i < lenLORA1; i++) {
          if ( j > lora_size) {
            j = 0;
            transmitLoRa(&LoRa1, thePacket, j);
          }
          thePacket[j] = LORA1buffer[i];
          j++;
        }
        transmitLoRa(&LoRa1, thePacket, j);
        lenLORA1 = 0;
        break;
      }
    case 2:
      {
        DataReading thePacket[lora_size];
        int j = 0;
        for (int i = 0; i < lenLORA2; i++) {
          if ( j > lora_size) {
            j = 0;
            transmitLoRa(&LoRa2, thePacket, j);
          }
          thePacket[j] = LORA2buffer[i];
          j++;
        }
        transmitLoRa(&LoRa2, thePacket, j);
        lenLORA2 = 0;

        break;
      }
  }
#endif
}

void releaseSerial() {
  DBG("Releasing Serial.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < lenSERIAL; i++) {
    doc[i]["id"]   = SERIALbuffer[i].id;
    doc[i]["type"] = SERIALbuffer[i].t;
    doc[i]["data"] = SERIALbuffer[i].d;
  }
  serializeJson(doc, UART_IF);
  UART_IF.println();
  lenSERIAL = 0;
}

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

void begin_espnow() {
#ifdef USE_ESPNOW
  DBG("Initializing ESP-NOW!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#if defined(ESP8266)
  wifi_set_macaddr(STATION_IF, selfAddress);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peers
  //#ifdef ESPNOW1_PEER
  //  esp_now_add_peer(ESPNOW1, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  //#endif
  //#ifdef ESPNOW2_PEER
  //  esp_now_add_peer(ESPNOW2, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  //#endif
#elif defined(ESP32)
  esp_wifi_set_mac(WIFI_IF_STA, &selfAddress[0]);
  if (esp_now_init() != ESP_OK) {
    DBG("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Register first peer

  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer bcast");
    return;
  }
  //#ifdef ESPNOW1_PEER
  //  memcpy(peerInfo.peer_addr, ESPNOW1, 6);
  //  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  //    DBG("Failed to add peer 1");
  //    return;
  //  }
  //#endif
  //#ifdef ESPNOW2_PEER
  //  memcpy(peerInfo.peer_addr, ESPNOW2, 6);
  //  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  //    DBG("Failed to add peer 2");
  //    return;
  //  }
  //#endif
#endif //ESP8266
  DBG(" ESP-NOW Initialized.");
#endif //USE_ESPNOW
}

void begin_lora() {
#ifdef USE_LORA
  DBG("Initializing LoRa!");
#ifdef ESP32
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
#endif
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(FDRS_BAND)) {
    DBG(" Initialization failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(FDRS_SF);
  DBG("LoRa Band: " + String(FDRS_BAND));
  DBG("LoRa SF  : " + String(FDRS_SF));
#endif // USE_LORA
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

void handleCommands() {
  switch (theCmd.cmd) {
    case cmd_ping:
      DBG("Ping back to sender");
      SystemPacket sys_packet;
      sys_packet.cmd = cmd_ping;
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
      break;
    case cmd_add:
      DBG("Add sender to peer list (not completed)");
      break;
  }
  theCmd.cmd = cmd_clear;
  theCmd.param = 0;
}

#endif //__FDRS_FUNCTIONS_H__