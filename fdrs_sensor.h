//  FARM DATA RELAY SYSTEM
//
//  "fdrs_sensor.h"
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
#include "fdrs_datatypes.h"
#include "fdrs_sensor_config.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#ifdef USE_LORA
#include <LoRa.h>
#endif

#ifdef FDRS_GLOBALS
#define FDRS_BAND GLOBAL_LORA_BAND
#define FDRS_SF GLOBAL_LORA_SF
#else
#define FDRS_BAND LORA_BAND
#define FDRS_SF LORA_SF
#endif

#ifdef FDRS_DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

enum crcResult{
  CRC_NULL,
  CRC_OK,
  CRC_BAD,
} returnCRC = CRC_NULL;

const uint16_t espnow_size = 250 / sizeof(DataReading);
uint8_t gatewayAddress[] = {MAC_PREFIX, GTWY_MAC};
uint16_t gtwyAddress = ((gatewayAddress[4] << 8) | GTWY_MAC);
uint16_t LoRaAddress = 0x4200;


uint32_t wait_time = 0;
DataReading fdrsData[espnow_size];
uint8_t data_count = 0;

void beginFDRS() {
#ifdef FDRS_DEBUG
  Serial.begin(115200);
#endif
  DBG("FDRS Sensor ID " + String(READING_ID) + " initializing...");
  DBG(" Gateway: " + String (GTWY_MAC, HEX));
#ifdef POWER_CTRL
  DBG("Powering up the sensor array!");
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
#endif
  // Init ESP-NOW for either ESP8266 or ESP32 and set MAC address
#ifdef USE_ESPNOW
  DBG("Initializing ESP-NOW!");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
#if defined(ESP8266)
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  // Register peers
  esp_now_add_peer(gatewayAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)
  if (esp_now_init() != ESP_OK) {
    DBG("Error initializing ESP-NOW");
    return;
  }
  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Register first peer
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer");
    return;
  }
#endif
  DBG(" ESP-NOW Initialized.");
#endif
#ifdef USE_LORA
  DBG("Initializing LoRa!");
  DBG(FDRS_BAND);
  DBG(FDRS_SF);
#ifdef ESP32
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
#endif
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(FDRS_BAND)) {
    DBG("Unable to initialize LoRa!");
    while (1);
  }
  LoRa.setSpreadingFactor(FDRS_SF);
  DBG(" LoRa Initialized.");
#endif
}

//  USED to get ACKs from LoRa gateway at this point.  May be used in the future to get other data
// Return true if ACK received otherwise False
// TODO need to handle NAK
crcResult getLoRaAck() {
#ifdef USE_LORA
  int packetSize = LoRa.parsePacket();
  if (packetSize) {  // TODO: check for max packet size??
    uint8_t packet[packetSize];
    uint16_t sourceMAC = 0x0000;
    uint16_t destMAC = 0x0000;
    uint16_t packetCRC = 0x0000; // CRC Extracted from received LoRa packet
    uint16_t calcCRC = 0x0000; // CRC calculated from received LoRa packet
    unsigned long ln = (packetSize - 6) / sizeof(DataReading);
    DataReading ackPacket[ln];  // I mean it shouldn't need to be this large but who knows what we're gonna get?  Do we need to check memory overrun?
  
    LoRa.readBytes((uint8_t *)&packet, packetSize);
    
    packetCRC = ((packet[packetSize - 2] << 8) | packet[packetSize - 1]);
    DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(LoRa.packetRssi()) + "dBi, SNR: " + String(LoRa.packetSnr()) + "dB, FreqError: " + String(LoRa.packetFrequencyError()) + "Hz, PacketCRC: 0x" + String(packetCRC,16));
    if (memcmp(&packet, &LoRaAddress, 2) == 0) {   //Check if addressed to this device (2 bytes)
      // TODO: Do we check if the ln == 1 && .t == CRC_T?  If so then we should not do any more processing as the packet is an ACK packet and there is no need
      memcpy(&destMAC, &packet[0], 2);             //Split off address portion of packet (2 bytes)
      memcpy(&sourceMAC, &packet[2], 2);             //Split off address portion of packet (2 bytes)
      memcpy(ackPacket, &packet[4], packetSize - 6);   //Split off data portion of packet (N bytes)
      // Calculate the received packet CRC
      if(packetCRC == 0xFFFF) {
        DBG("Sensor address 0x" + String(sourceMAC,16) + "(hex) does not want ACK");
        return packet_NOACK;
      }
      else {
        for(int i = 0; i < (packetSize - 2); i++) { // Last 2 bytes of packet are the CRC so do not include them in calculation
          calcCRC = crc16_update(calcCRC, packet[i]);
        }
        if(calcCRC == packetCRC) {
          // TODO: Queue up or Send ACK
          // Create ACK DataReading Struct
          // DataReading ACK = { .d = 1, // set this to something else????
          //                     .id = destMAC,
          //                     .t = CRC_T };
          DBG("CRC Match, sending ACK packet to sensor 0x" + String(sourceMAC,16) + "(hex)");
          //transmitLoRa(loraGwAddress, &ACK, 1);
          return packet_CRCOK;
        }
        else {
          // DataReading NAK = { .d = -1, // set this to something else???
          //                     .id = destMAC,
          //                     .t = CRC_T };
          
          // Send NAK packet to sensor
          DBG("CRC Mismatch! Packet CRC is 0x" + String(packetCRC,16) + ", Calculated CRC is 0x" + String(calcCRC,16) + " Sending NAK packet to sensor 0x" + String(sourceMAC,16) + "(hex)");
          //transmitLoRa(loraGwAddress, &NAK, 1);
          return packet_CRCBAD;
        }
      }
    }
    else {
      DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received, not destined to our gateway.");
      return packet_NULL;
    }
  }
#endif
}

void transmitLoRa(uint16_t* destMAC, DataReading * packet, uint8_t len) {
#ifdef USE_LORA
  uint8_t pkt[6 + (len * sizeof(DataReading))];
  uint16_t calcCRC;

  memcpy(&pkt[0], destMAC, 2);  //
  memcpy(&pkt[2], &LoRaAddress, 2);
  memcpy(&pkt[4], packet, len * sizeof(DataReading));
  // IF ACKs are disabled set CRC to fixed value (0xFFFF??)
  for(int i = 0; i < (sizeof(pkt) - 2); i++) {  // Last 2 bytes are CRC so do not include them in the calculation itself
    calcCRC = crc16_update(calcCRC, pkt[i]);
  }
  memcpy(&pkt[(len * sizeof(DataReading) + 4)], &calcCRC, 2);
  //pkt[(len * sizeof(DataReading) + 4)] = calcCRC; // Append calculated CRC to the last 2 bytes of the packet
  DBG("Transmitting LoRa of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC,16));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
#endif
}

void sendFDRS() {
  unsigned long loraAckTimeout;

  DBG("Sending FDRS Packet!");
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *) &fdrsData, data_count * sizeof(DataReading));
  delay(5);
  DBG(" ESP-NOW sent.");
#endif
#ifdef USE_LORA
  transmitLoRa(&gtwyAddress, fdrsData, data_count);
  DBG(" LoRa sent.");
  loraAckTimeout = millis() + 2000; // 2000ms timeout waiting for LoRa ACK
  while((returnCRC != packet_NULL) && (millis() < loraAckTimeout)) {
    returnCRC = getLoRaAck();
  }
  if(returnCRC == packet_ACK) {
    DBG("LoRa ACK Received!");
  }
  else {
    DBG("LoRa Timeout waiting for ACK!");
  }
#endif
  data_count = 0;
  returnCRC = CRC_NULL;
}
void loadFDRS(float d, uint8_t t) {
  DBG("Data loaded. Type: " + String(t));
  if (data_count > espnow_size) sendFDRS();
  DataReading dr;
  dr.id = READING_ID;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}
void sleepFDRS(int sleep_time) {
  DBG("Sleepytime!");
#ifdef DEEP_SLEEP
  DBG(" Deep sleeping.");
#ifdef ESP32
  esp_sleep_enable_timer_wakeup(sleep_time * 1000000);
  esp_deep_sleep_start();
#endif
#ifdef ESP8266
  ESP.deepSleep(sleep_time * 1000000);
#endif
#endif
  DBG(" Delaying.");
  delay(sleep_time * 1000);
}


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