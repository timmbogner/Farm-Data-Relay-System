//  FARM DATA RELAY SYSTEM
//
//  "fdrs_sensor.h"
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
#include <fdrs_datatypes.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif
#ifdef USE_LORA
#include <ArduinoUniqueID.h>
#include <LoRa.h>
#endif

// enable to get detailed info from where single configuration macros have been taken
#define DEBUG_CONFIG
#define LORA_ACK_TIMEOUT 400    // LoRa ACK timeout in ms. (Minimum = 200)
#define LORA_RETRIES 2          // LoRa ACK automatic retries [0 - 3]

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

#ifdef FDRS_DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

#ifdef DEBUG_CONFIG
//#include "fdrs_checkConfig.h"
#endif

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;


enum crcResult {
  CRC_NULL,
  CRC_OK,
  CRC_BAD,
} returnCRC;

enum {
  cmd_clear,
  cmd_ping,
  cmd_add,
  cmd_ack,
};

typedef struct __attribute__((packed)) SystemPacket {
  uint8_t cmd;
  uint32_t param;
} SystemPacket;

const uint16_t espnow_size = 250 / sizeof(DataReading);
uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t gatewayAddress[] = {MAC_PREFIX, GTWY_MAC};
uint16_t gtwyAddress = ((gatewayAddress[4] << 8) | GTWY_MAC);
#ifdef USE_LORA
uint16_t LoRaAddress = ((UniqueID8[6] << 8) | UniqueID8[7]);
unsigned long transmitLoRaMsg = 0;  // Number of total LoRa packets destined for us and of valid size
unsigned long msgOkLoRa = 0;     // Number of total LoRa packets with valid CRC
#endif 
uint32_t gtwy_timeout = 0;
uint8_t incMAC[6];
uint32_t wait_time = 0;
DataReading fdrsData[espnow_size];
DataReading incData[espnow_size];
crcResult esp_now_ack_flag;

uint8_t data_count = 0;
bool is_ping = false;
bool is_added = false;
uint32_t last_refresh;
void (*callback_ptr)(DataReading);
uint16_t subscription_list[256] = {};
bool active_subs[256] = {};

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if(sendStatus == 0){
    esp_now_ack_flag = CRC_OK;
  } else {
    esp_now_ack_flag = CRC_BAD;
  }
}
void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status == ESP_NOW_SEND_SUCCESS){
    esp_now_ack_flag = CRC_OK;
  } else {
    esp_now_ack_flag = CRC_BAD;
  }  
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif

  memcpy(&incMAC, mac, sizeof(incMAC));

  if (len < sizeof(DataReading)) {
    SystemPacket command;
    memcpy(&command, incomingData, sizeof(command));
    switch (command.cmd) {
      case cmd_ping:
        is_ping = true;
        break;
      case cmd_add:
        is_added = true;
        gtwy_timeout = command.param;
        break;
    }
  } else{
    memcpy(&incData, incomingData, len);
  int pkt_readings = len / sizeof(DataReading);
  for (int i = 0; i <= pkt_readings; i++) {        //Cycle through array of incoming DataReadings for any addressed to this device
    for (int j = 0; j < 255; j++){                 //Cycle through subscriptions for active entries
      if (active_subs[j]){
        if (incData[i].id == subscription_list[j]){
          (*callback_ptr)(incData[i]);
          }
        }
      }
    }
  }
}


void beginFDRS() {
#ifdef FDRS_DEBUG
  Serial.begin(115200);
  // // find out the reset reason
  // esp_reset_reason_t resetReason;
  // resetReason = esp_reset_reason();
#endif
  DBG("FDRS User Node initializing...");
  DBG(" Reading ID " + String(READING_ID));
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
  esp_now_register_recv_cb(OnDataRecv);

  // Register peers
  esp_now_add_peer(gatewayAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)

  if (esp_now_init() != ESP_OK) {
    DBG("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer bcast");
    return;
  }
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer");
    return;
  }
#endif
  DBG(" ESP-NOW Initialized.");
#endif //USE_ESPNOW
#ifdef USE_LORA
  DBG("Initializing LoRa!");
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

  DBG("LoRa Band: " + String(FDRS_BAND));
  DBG("LoRa SF  : " + String(FDRS_SF));
#endif // USE_LORA
#ifdef DEBUG_CONFIG
  // if (resetReason != ESP_RST_DEEPSLEEP) {
    //checkConfig();
  // }
#endif //DEBUG_CONFIG

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

static uint16_t crc16_update(uint16_t crc, uint8_t a) {
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

//  USED to get ACKs from LoRa gateway at this point.  May be used in the future to get other data
// Return type is crcResult struct - CRC_OK, CRC_BAD, CRC_NULL.  CRC_NULL used for non-ack data
crcResult getLoRa() {
#ifdef USE_LORA
  int packetSize = LoRa.parsePacket();
  if ((packetSize - 6) % sizeof(SystemPacket) == 0 && packetSize > 0) {  // packet size should be 6 bytes plus multiple of size of SystemPacket
    uint8_t packet[packetSize];
    uint16_t sourceMAC = 0x0000;
    uint16_t destMAC = 0x0000;
    uint16_t packetCRC = 0x0000; // CRC Extracted from received LoRa packet
    uint16_t calcCRC = 0x0000; // CRC calculated from received LoRa packet
    uint ln = (packetSize - 6) / sizeof(SystemPacket);
    SystemPacket receiveData[ln];

    LoRa.readBytes((uint8_t *)&packet, packetSize);

    destMAC = (packet[0] << 8) | packet[1];
    sourceMAC = (packet[2] << 8) | packet[3];
    packetCRC = ((packet[packetSize - 2] << 8) | packet[packetSize - 1]);
    DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(LoRa.packetRssi()) + "dBi, SNR: " + String(LoRa.packetSnr()) + "dB, PacketCRC: 0x" + String(packetCRC, 16));
    if (destMAC == LoRaAddress) {
      //printLoraPacket(packet,sizeof(packet));
      memcpy(receiveData, &packet[4], packetSize - 6);   //Split off data portion of packet (N bytes)
      if (ln == 1 && receiveData[0].cmd == cmd_ack) { // We have received an ACK packet
        if (packetCRC == 0xFFFF) {
          DBG("ACK Received - address 0x" + String(sourceMAC, 16) + "(hex) does not want ACKs");
          return CRC_OK;
        }
        else {
          for (int i = 0; i < (packetSize - 2); i++) { // Last 2 bytes of packet are the CRC so do not include them in calculation
            //printf("CRC: %02X : %d\n",calcCRC, i);
            calcCRC = crc16_update(calcCRC, packet[i]);
          }
          if (calcCRC == packetCRC) {
            DBG("ACK Received - CRC Match");
            return CRC_OK;
          }
          else {
            DBG("ACK Received CRC Mismatch! Packet CRC is 0x" + String(packetCRC, 16) + ", Calculated CRC is 0x" + String(calcCRC, 16));
            return CRC_BAD;
          }
        }
      }
      else { // data we have received is not of type ACK_T.  How we handle is future enhancement.
        DBG("Received some LoRa SystemPacket data that is not of type ACK.  To be handled in future enhancement.");
        DBG("ln: " + String(ln) + "data type: " + String(receiveData[0].cmd));
        return CRC_NULL;
      }
    }
    else if ((packetSize - 6) % sizeof(DataReading) == 0) { // packet size should be 6 bytes plus multiple of size of DataReading)
      DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received, with DataReading data to be processed.");
      return CRC_NULL;
    }
    else {
      DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received, not destined to our address.");
      return CRC_NULL;
    }
  }
  else {
    if (packetSize != 0) {
      DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received");
    }
  }
  return CRC_NULL;
#endif
}

void printLoraPacket(uint8_t* p, int size) {
  printf("Printing packet of size %d.", size);
  for (int i = 0; i < size; i++ ) {
    if (i % 2 == 0) printf("\n%02d: ", i);
    printf("%02X ", p[i]);
  }
  printf("\n");
}

bool transmitLoRa(uint16_t* destMAC, DataReading * packet, uint8_t len) {
#ifdef USE_LORA
  uint8_t pkt[6 + (len * sizeof(DataReading))];
  uint16_t calcCRC = 0x0000;

  pkt[0] = (*destMAC >> 8);
  pkt[1] = (*destMAC & 0x00FF);
  pkt[2] = (LoRaAddress >> 8);
  pkt[3] = (LoRaAddress & 0x00FF);
  memcpy(&pkt[4], packet, len * sizeof(DataReading));
  for (int i = 0; i < (sizeof(pkt) - 2); i++) { // Last 2 bytes are CRC so do not include them in the calculation itself
    //printf("CRC: %02X : %d\n",calcCRC, i);
    calcCRC = crc16_update(calcCRC, pkt[i]);
  }
#ifndef LORA_ACK
  calcCRC = crc16_update(calcCRC, 0xA1); // Recalculate CRC for No ACK
#endif    // LORA_ACK
  pkt[len * sizeof(DataReading) + 4] = (calcCRC >> 8);
  pkt[len * sizeof(DataReading) + 5] = (calcCRC & 0x00FF);
#ifdef LORA_ACK  // Wait for ACK
  int retries = LORA_RETRIES + 1;
  while (retries != 0) {
    if (transmitLoRaMsg != 0)
      DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, 16) + " to gateway 0x" + String(*destMAC, 16) + ". Retries remaining: " + String(retries - 1) + ", CRC OK " + String((float)msgOkLoRa / transmitLoRaMsg * 100) + "%");
    else
      DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, 16) + " to gateway 0x" + String(*destMAC, 16) + ". Retries remaining: " + String(retries - 1));
    //printLoraPacket(pkt,sizeof(pkt));
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pkt, sizeof(pkt));
    LoRa.endPacket();
    transmitLoRaMsg++;
    unsigned long loraAckTimeout = millis() + LORA_ACK_TIMEOUT;
    retries--;
    delay(10);
    while (returnCRC == CRC_NULL && (millis() < loraAckTimeout)) {
      returnCRC = getLoRa();
    }
    if (returnCRC == CRC_OK) {
      //DBG("LoRa ACK Received! CRC OK");
      msgOkLoRa++;
      return true;  // we're done
    }
    else if (returnCRC == CRC_BAD) {
      //DBG("LoRa ACK Received! CRC BAD");
      // Resend original packet again if retries are available
    }
    else {
      DBG("LoRa Timeout waiting for ACK!");
      // resend original packet again if retries are available
    }  
  }
  return false;
#else   // Send and do not wait for ACK reply
  DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, 16) + " to gateway 0x" + String(*destMAC, 16));
  //printLoraPacket(pkt,sizeof(pkt));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
  transmitLoRaMsg++;
  return true;
#endif    // LORA_ACK
#endif    // USE_LORA
}

bool sendFDRS() {
  DBG("Sending FDRS Packet!");
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *) &fdrsData, data_count * sizeof(DataReading));
  esp_now_ack_flag =  CRC_NULL;
  while(esp_now_ack_flag == CRC_NULL){
    delay(0);
  }
  if (esp_now_ack_flag == CRC_OK){
      return true;
    } else {
      return false;
    }

#endif
#ifdef USE_LORA
  if(transmitLoRa(&gtwyAddress, fdrsData, data_count)){
    data_count = 0;
    returnCRC = CRC_NULL;
    return true;
  } else {
    data_count = 0;
    returnCRC = CRC_NULL;
    return false;
  }
#endif
return false;
}

void loadFDRS(float d, uint8_t t) {
  DBG("Id: " + String(READING_ID) + " - Type: " + String(t) + " - Data loaded: " + String(d));
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


void loopFDRS() {
  if (is_added) {
    if ((millis() - last_refresh) >= gtwy_timeout) {
      last_refresh  = millis();
    }
  }
}




bool seekFDRS(int timeout) {
  SystemPacket sys_packet = { .cmd = cmd_ping, .param = 0 };
#ifdef USE_ESPNOW
  esp_now_send(broadcast_mac, (uint8_t *) &sys_packet, sizeof(SystemPacket));
  DBG("Seeking nearby gateways");
  uint32_t ping_start = millis();
  is_ping = false;
  while ((millis() - ping_start) <= timeout) {
    yield(); //do I need to yield or does it automatically?
    if (is_ping) {
      DBG("Responded:" + String(incMAC[5]));
      return true;
    }
  }
  return false;

#endif
}

bool addFDRS(int timeout, void (*new_cb_ptr)(DataReading)) {
    
    callback_ptr = new_cb_ptr;
    
    SystemPacket sys_packet = { .cmd = cmd_add, .param = 0 };
    #ifdef USE_ESPNOW
    esp_now_send(gatewayAddress, (uint8_t *) &sys_packet, sizeof(SystemPacket));
    DBG("ESP-NOW peer registration request submitted to " + String(gatewayAddress[5]));
    uint32_t add_start = millis();
    is_added = false;
    while ((millis() - add_start) <= timeout) { 
       yield();
      if (is_added) {
        DBG("Registration accepted. Timeout: " + String(gtwy_timeout));
        last_refresh = millis();
        return true;
      }
    }
    DBG("No gateways accepted the request");
    return false;
    #endif
  }

uint32_t pingFDRS(int timeout) {
  SystemPacket sys_packet = { .cmd = cmd_ping, .param = 0 };
#ifdef USE_ESPNOW
  esp_now_send(gatewayAddress, (uint8_t *) &sys_packet, sizeof(SystemPacket));
  DBG(" ESP-NOW ping sent.");
  uint32_t ping_start = millis();
  is_ping = false;
  while ((millis() - ping_start) <= timeout) {
    yield(); //do I need to yield or does it automatically?
    if (is_ping) {
      DBG("Ping Returned:" + String(millis() - ping_start) + " from " + String(incMAC[5]));
      return millis() - ping_start;
    }
  }
#endif
#ifdef USE_LORA
  //transmitLoRa(gtwyAddress, sys_packet, data_count); // TODO: Make this congruent to esp_now_send()
  DBG(" LoRa ping not sent because it isn't implemented.");
#endif
}
bool subscribeFDRS(uint16_t sub_id){
  for(int i = 0; i < 255; i++){
    if ((subscription_list[i] == sub_id) && (active_subs[i])){
      DBG("You're already subscribed to that!");
      return true;
    }
  }
  for(int i = 0; i < 255; i++){
    if (!active_subs[i]){
      DBG("Adding subscription at position " + String(i));
        subscription_list[i] = sub_id;
        active_subs[i] = true;
        return true;
        
    }
    
  }
  DBG("No subscription could be established!");
  return false;

}
bool unsubscribeFDRS(uint16_t sub_id){
  for(int i = 0; i < 255; i++){
    if ((subscription_list[i] == sub_id) && (active_subs[i])){
      DBG("Removing subscription.");
      active_subs[i] = false;
      return true;
    }
  }
  DBG("No subscription to remove");
  return false;

}
