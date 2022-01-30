const uint8_t espnow_size = 250 / sizeof(DataReading);
const uint8_t lora_size   = 256 / sizeof(DataReading);

uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t ESPNOW1[] =       {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, ESPNOW1_MAC};
uint8_t selfAddress[] =   {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t ESPNOW2[] =       {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, ESPNOW2_MAC};
uint8_t incMAC[6];

DataReading theData[256];
uint8_t ln;
uint8_t newData = 0;

DataReading bufferESPNOW1[256];
uint8_t lenESPNOW1 = 0;
uint32_t timeESPNOW1 = 0;
DataReading bufferESPNOW2[256];
uint8_t lenESPNOW2 = 0;
uint32_t timeESPNOW2 = 0;
DataReading bufferESPNOWG[256];
uint8_t lenESPNOWG = 0;
uint32_t timeESPNOWG = 0;
DataReading bufferSERIAL[256];
uint8_t lenSERIAL = 0;
uint32_t timeSERIAL = 0;
DataReading bufferMQTT[256];
uint8_t lenMQTT = 0;
uint32_t timeMQTT = 0;
DataReading bufferLORA[256];
uint8_t lenLORA = 0;
uint32_t timeLORA = 0;


WiFiClient espClient;
PubSubClient client(espClient);

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
  memcpy(&theData, incomingData, sizeof(theData));
  memcpy(&incMAC, mac, sizeof(incMAC));
  if (memcmp(&incMAC, &ESPNOW1, 6) == 0) newData = 1;
  else if (memcmp(&incMAC, &ESPNOW2, 6) == 0) newData = 2;
  else newData = 3;
  ln = len / sizeof(DataReading);
  Serial.println("RCV:" + String(ln));
}
void getSerial() {
  String incomingString =  Serial.readStringUntil('\n');
  DynamicJsonDocument doc(24576);
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    Serial.println("parse err");
    return;
  } else {
    int s = doc.size();
    //Serial.println(s);
    for (int i = 0; i < s; i++) {
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = 4;
  }
}
void mqtt_callback(char* topic, byte * message, unsigned int length) {
  String incomingString;
  for (int i = 0; i < length; i++) {
    incomingString += (char)message[i];
  }
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    //Serial.println("parse err");
    return;
  } else {
    int s = doc.size();
    //Serial.println(s);
    for (int i = 0; i < s; i++) {
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = 5;
  }
}
#ifdef USE_LORA
void getLoRa() {
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    LoRa.readBytes((uint8_t *)&theData, packetSize);
    ln = packetSize / sizeof(DataReading);
    newData = 6;
  }
}
#endif


void sendESPNOW(uint8_t interface) {
  switch (interface) {
    case 0:
      for (int i = 0; i < ln; i++) {
        bufferESPNOWG[lenESPNOWG + i] = theData[i];
      }
      lenESPNOWG +=  ln;
      break;
    case 1:
      for (int i = 0; i < ln; i++) {
        bufferESPNOW1[lenESPNOW1 + i] = theData[i];
      }
      lenESPNOW1 +=  ln;
      break;
    case 2:
      for (int i = 0; i < ln; i++) {
        bufferESPNOW2[lenESPNOW2 + i] = theData[i];
      }
      lenESPNOW2 +=  ln;
      break;
  }
}
void sendSerial() {
  for (int i = 0; i < ln; i++) {
    bufferSERIAL[lenSERIAL + i] = theData[i];
  }
  lenSERIAL += ln;
  Serial.println("SENDSERIAL:" + String(lenSERIAL)+" ");

}
void sendMQTT() {
  for (int i = 0; i < ln; i++) {
    bufferMQTT[lenMQTT + i] = theData[i];
  }
  lenMQTT += ln;
}
void sendLoRa() {
  for (int i = 0; i < ln; i++) {
    bufferLORA[lenLORA + i] = theData[i];
  }
  lenLORA += ln;
}

void releaseESPNOW(uint8_t interface) {
  switch (interface) {
    case 0:
      {
        DataReading thePacket[espnow_size];
        int j = 0;
        for (int i = 0; i < lenESPNOWG; i++) {
          if ( j > 250 / sizeof(DataReading)) {
            j = 0;
            esp_now_send(broadcast_mac, (uint8_t *) &thePacket, sizeof(thePacket));
          }
          thePacket[j] = bufferESPNOWG[i];
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
          if ( j > 250 / sizeof(DataReading)) {
            j = 0;
            esp_now_send(ESPNOW1, (uint8_t *) &thePacket, sizeof(thePacket));
          }
          thePacket[j] = bufferESPNOW1[i];
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
          if ( j > 250 / sizeof(DataReading)) {
            j = 0;
            esp_now_send(ESPNOW2, (uint8_t *) &thePacket, sizeof(thePacket));
          }
          thePacket[j] = bufferESPNOW2[i];
          j++;
        }
        esp_now_send(ESPNOW2, (uint8_t *) &thePacket, j * sizeof(DataReading));
        lenESPNOW2 = 0;
        break;
      }
  }
}
void releaseSerial() {
  //DynamicJsonDocument doc(24576);
    StaticJsonDocument<2048> doc;

  for (int i = 0; i < lenSERIAL; i++) {
    doc[i]["id"]   = bufferSERIAL[i].id;
    doc[i]["type"] = bufferSERIAL[i].t;
    doc[i]["data"] = bufferSERIAL[i].d;
  }
  serializeJson(doc, Serial);
  Serial.println();
  lenSERIAL = 0;
}
void releaseMQTT() {
#ifdef USE_WIFI
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < lenMQTT; i++) {
    doc[i]["id"]   = bufferMQTT[i].id;
    doc[i]["type"] = bufferMQTT[i].t;
    doc[i]["data"] = bufferMQTT[i].d;
  }
  String outgoingString;
  serializeJson(doc, outgoingString);
  client.publish("esp/fdrs", (char*) outgoingString.c_str());
  lenMQTT = 0;
#endif
}
void releaseLoRa() {
#ifdef USE_LORA
  DataReading thePacket[lora_size];
  int j = 0;
  for (int i = 0; i < lenLORA); i++) {
    if ( j > lora_size)) {
      j = 0;
      LoRa.beginPacket();
      LoRa.write((uint8_t*)&thePacket, j * sizeof(DataReading));
      LoRa.endPacket();
    }
    thePacket[j] = bufferLORA[i];
    j++;
  }
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&thePacket, j * sizeof(DataReading));
  LoRa.endPacket();
  lenLORA = 0;

#endif
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("FDRS_GATEWAY")) {
      // Subscribe
      client.subscribe("esp/fdrs");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void begin_espnow() {
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
  esp_now_add_peer(ESPNOW1, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  esp_now_add_peer(ESPNOW2, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)
  esp_wifi_set_mac(WIFI_IF_STA, &selfAddress[0]);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Register first peer
  memcpy(peerInfo.peer_addr, ESPNOW1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, ESPNOW2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, broadcast_mac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
}
