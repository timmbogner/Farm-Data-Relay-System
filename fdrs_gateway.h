#ifdef DEBUG
#define DBG(a) (Serial.println(a))
#else
#define DBG(a)
#endif

#if defined (ESP32)
#define UART_IF Serial1
#else
#define UART_IF Serial
#endif

#ifdef GLOBALS
#define FDRS_WIFI_SSID GLOBAL_SSID
#define FDRS_WIFI_PASS GLOBAL_PASS
#define FDRS_MQTT_ADDR GLOBAL_MQTT_ADDR
#define FDRS_BAND GLOBAL_BAND
#define FDRS_SF GLOBAL_SF
#else
#define FDRS_WIFI_SSID WIFI_SSID
#define FDRS_WIFI_PASS WIFI_PASS
#define FDRS_MQTT_ADDR MQTT_ADDR
#define FDRS_BAND BAND
#define FDRS_SF SF
#endif

#define USE_LORA

#define MAC_PREFIX  0xAA, 0xBB, 0xCC, 0xDD, 0xEE  // Should only be changed if implementing multiple FDRS systems.

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

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
uint8_t LoRa1[] =         {mac_prefix[3], mac_prefix[4], LORA1_PEER};
uint8_t LoRa2[] =         {mac_prefix[3], mac_prefix[4], LORA2_PEER};
//uint8_t LoRaAddress[] = {0x42, 0x00};
#endif

DataReading ESPNOW1buffer[256];
uint8_t lenESPNOW1 = 0;
uint32_t timeESPNOW1 = 0;
DataReading ESPNOW2buffer[256];
uint8_t lenESPNOW2 = 0;
uint32_t timeESPNOW2 = 0;
DataReading ESPNOWGbuffer[256];
uint8_t lenESPNOWG = 0;
uint32_t timeESPNOWG = 0;
DataReading SERIALbuffer[256];
uint8_t lenSERIAL = 0;
uint32_t timeSERIAL = 0;
DataReading MQTTbuffer[256];
uint8_t lenMQTT = 0;
uint32_t timeMQTT = 0;
DataReading LORAGbuffer[256];
uint8_t lenLORAG = 0;
uint32_t timeLORAG = 0;
DataReading LORA1buffer[256];
uint8_t lenLORA1 = 0;
uint32_t timeLORA1 = 0;
DataReading LORA2buffer[256];
uint8_t lenLORA2 = 0;
uint32_t timeLORA2 = 0;

WiFiClient espClient;
#ifdef USE_LED
CRGB leds[NUM_LEDS];
#endif
#ifdef USE_WIFI
PubSubClient client(espClient);
const char* ssid = FDRS_WIFI_SSID;
const char* password = FDRS_WIFI_PASS;
const char* mqtt_server = FDRS_MQTT_ADDR;
#endif

void getSerial(void);


void mqtt_callback(char* topic, byte * message, unsigned int length);

void getLoRa();

void sendESPNOW(uint8_t address);

void sendSerial();

void sendMQTT() {
#ifdef USE_WIFI
  DBG("Sending MQTT.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < ln; i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
  }
  String outgoingString;
  serializeJson(doc, outgoingString);
  client.publish(TOPIC_DATA, (char*) outgoingString.c_str());
#endif
}

void bufferESPNOW(uint8_t interface) {
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
}

void releaseESPNOW(uint8_t interface) {
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
}
#ifdef USE_LORA
void transmitLoRa(uint8_t* mac, DataReading * packet, uint8_t len) {
  DBG("Transmitting LoRa.");

  uint8_t pkt[5 + (len * sizeof(DataReading))];
  memcpy(&pkt, mac, 3);
  memcpy(&pkt[3], &selfAddress[4], 2);
  memcpy(&pkt[5], packet, len * sizeof(DataReading));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
}
#endif

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
            transmitLoRa(broadcast_mac, thePacket, j);
          }
          thePacket[j] = LORAGbuffer[i];
          j++;
        }
        transmitLoRa(broadcast_mac, thePacket, j);
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
            transmitLoRa(LoRa1, thePacket, j);
          }
          thePacket[j] = LORA1buffer[i];
          j++;
        }
        transmitLoRa(LoRa1, thePacket, j);
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
            transmitLoRa(LoRa2, thePacket, j);
          }
          thePacket[j] = LORA2buffer[i];
          j++;
        }
        transmitLoRa(LoRa2, thePacket, j);
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
  client.publish(TOPIC_DATA, (char*) outgoingString.c_str());
  lenMQTT = 0;
#endif
}
void reconnect() {
#ifdef USE_WIFI
  // Loop until reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("FDRS_GATEWAY")) {
      // Subscribe
      client.subscribe(TOPIC_COMMAND);
    } else {
      DBG("Connecting MQTT.");
      delay(5000);
    }
  }
#endif
}
void begin_espnow() {
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
#ifdef ESPNOW1_PEER
  esp_now_add_peer(ESPNOW1, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#endif
#ifdef ESPNOW2_PEER
  esp_now_add_peer(ESPNOW2, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#endif
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
#ifdef ESPNOW1_PEER
  memcpy(peerInfo.peer_addr, ESPNOW1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer 1");
    return;
  }
#endif
#ifdef ESPNOW2_PEER
  memcpy(peerInfo.peer_addr, ESPNOW2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    DBG("Failed to add peer 2");
    return;
  }
#endif
#endif
  DBG(" ESP-NOW Initialized.");
}
