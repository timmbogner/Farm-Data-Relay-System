uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};
uint8_t nextAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, NEXT_MAC};
uint8_t incMAC[6];

DataReading theData[31];
uint8_t ln;
uint8_t newData = 0;

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
  if (memcmp(&incMAC, &prevAddress, 6) == 0) newData = 1;
  else if (memcmp(&incMAC, &nextAddress, 6) == 0) newData = 2;
  else newData = 3;
  ln = len;
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
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  esp_now_add_peer(nextAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
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
  memcpy(peerInfo.peer_addr, prevAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, nextAddress, 6);
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
void getSerial() {
  String incomingString =  Serial.readStringUntil('\n');
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, incomingString);
  if (error) {    // Test if parsing succeeds.
    Serial.println("parse err");
    return;
  } else {
    int s = doc.size();
    //Serial.println(s);
    for (int i = 0; i < s; i++) {
      if (i > 31) break;
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s * sizeof(DataReading);
    newData = 4;
  }
}

void sendESPNOW(uint8_t interface) {
  switch (interface) {
    case 0:
      esp_now_send(broadcast_mac, (uint8_t *) &theData, ln);
      break;
    case 1:
      esp_now_send(prevAddress, (uint8_t *) &theData, ln);
      break;
    case 2:
      esp_now_send(nextAddress, (uint8_t *) &theData, ln);
      break;
  }

}
void sendMQTT() {
#ifdef USE_WIFI
  StaticJsonDocument<2048> doc;
  for (int i = 0; i < ln / sizeof(DataReading); i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
  }
  String incomingString;
  serializeJson(doc, incomingString);
  client.publish("esp/fdrs", (char*) incomingString.c_str());
#endif
}

void sendSerial() {
  StaticJsonDocument<2048> doc;
  for (int i = 0; i < ln / sizeof(DataReading); i++) {
    doc[i]["id"]   = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
  }
  serializeJson(doc, Serial);
  Serial.println();
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
      if (i > 31) break;
      theData[i].id = doc[i]["id"];
      theData[i].t = doc[i]["type"];
      theData[i].d = doc[i]["data"];
    }
    ln = s * sizeof(DataReading);
    newData = 5;
  }
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
