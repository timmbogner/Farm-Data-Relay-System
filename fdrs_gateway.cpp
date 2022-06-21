#include "fdrs_gateway.h"

// #define ESP8266
#define ESP32

#define USE_WIFI

uint8_t newData = 0;
uint8_t ln = 0;
DataReading theData[256];

DataReadingBuffer_t ESPNOWGbuffer;

DataReadingBuffer_t ESPNOW1buffer;
uint32_t timeESPNOW1 = 0;

DataReadingBuffer_t ESPNOW2buffer;
uint32_t timeESPNOW2 = 0;

DataReadingBuffer_t SERIALbuffer;
uint32_t timeSERIAL = 0;

DataReadingBuffer_t MQTTbuffer;
uint32_t timeMQTT = 0;

DataReadingBuffer_t LORAGbuffer;
uint32_t timeLORAG = 0;

DataReadingBuffer_t LORA1buffer;
uint32_t timeLORA1 = 0;

DataReadingBuffer_t LORA2buffer;
uint32_t timeLORA2 = 0;

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32

#if defined(ESP8266)
void ESP8266OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {

}

void ESP8266OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
    OnDataRecv((uint8_t*)mac,*(const uint8_t *)incomingData,len);
}
#endif

#if defined(ESP32)
void ESP32OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

}

void ESP32OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    OnDataRecv((uint8_t*)mac,incomingData,len)
}
#endif

void OnDataRecv(uint8_t * mac, const uint8_t *incomingData, int len){

    memcpy(&theData, incomingData, sizeof(theData));
    memcpy(&incMAC, mac, sizeof(incMAC));
    DBG("Incoming ESP-NOW.");
    ln = len / sizeof(DataReading);

    if (memcmp(&incMAC, &ESPNOW1, 6) == 0){
        newData = 1;
        return;
    }

    if (memcmp(&incMAC, &ESPNOW2, 6) == 0){
        newData = 2;
        return;
    } 

    newData = 3;
}

void getSerial() {
  String incomingString =  UART_IF.readStringUntil('\n');
  DynamicJsonDocument doc(24576);
  DeserializationError error = deserializeJson(doc, incomingString);
    // Test if parsing succeeds.
    if (error) {    
    // DBG("json parse err");
    // DBG(incomingString);
        return;
    }

    int s = doc.size();
    //UART_IF.println(s);
    for (int i = 0; i < s; i++) {
        theData[i].id = doc[i]["id"];
        theData[i].t = doc[i]["type"];
        theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = 4;
    DBG("Incoming Serial.");
}


void mqtt_callback(char* topic, byte * message, unsigned int length) {
    String incomingString;
    DBG(topic);
    for (int i = 0; i < length; i++) {
        incomingString += (char)message[i];
    }
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, incomingString);
    if (error) {    // Test if parsing succeeds.
        DBG("json parse err");
        DBG(incomingString);
        return;
    }
    int s = doc.size();
    //UART_IF.println(s);
    for (int i = 0; i < s; i++) {
        theData[i].id = doc[i]["id"];
        theData[i].t = doc[i]["type"];
        theData[i].d = doc[i]["data"];
    }
    ln = s;
    newData = 5;
    DBG("Incoming MQTT.");
}

void getLoRa() {
#ifdef USE_LORA
    int packetSize = LoRa.parsePacket();
    if (packetSize== 0) {
        return;
    }
    uint8_t packet[packetSize];
    uint8_t incLORAMAC[2];
    LoRa.readBytes((uint8_t *)&packet, packetSize);
    //    for (int i = 0; i < packetSize; i++) {
    //      UART_IF.println(packet[i], HEX);
    //    }

    //Check if addressed to this device
    if (memcmp(&packet, &selfAddress[3], 3) != 0) { 
        return;    
    }

    memcpy(&incLORAMAC, &packet[3], 2);                  //Split off address portion of packet
    memcpy(&theData, &packet[5], packetSize - 5);        //Split off data portion of packet

    //Check if it is from a registered sender
    if(memcmp(&incLORAMAC, &LoRa1, 2) == 0){ 
        newData = 7;
    }     
    else if(memcmp(&incLORAMAC, &LoRa2, 2) == 0){
        newData = 8;
    }

    newData = 6;
    ln = (packetSize - 5) / sizeof(DataReading);
    
    DBG("Incoming LoRa.");
#endif
}

void sendESPNOW(uint8_t address) {
  DBG("Sending ESP-NOW.");
  uint8_t NEWPEER[] = {MAC_PREFIX, address};
#if defined(ESP32)
  esp_now_peer_info_t peerInfo;
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, NEWPEER, 6);
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
      esp_now_send(NEWPEER, (uint8_t *) &thePacket, sizeof(thePacket));
    }
    thePacket[j] = theData[i];
    j++;
  }
  esp_now_send(NEWPEER, (uint8_t *) &thePacket, j * sizeof(DataReading));
  esp_now_del_peer(NEWPEER);
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
        memcpy(&ESPNOWGbuffer.buffer[eSPNOWGbuffer.len],&theData[0],ln);
        eSPNOWGbuffer.len +=  ln;       
        break;
    case 1:
        memcpy(&ESPNOW1buffer.buffer[eSPNOW1buffer.len],&theData[0],ln);
        eSPNOW1buffer.len +=  ln;
        break;
    case 2:
        memcpy(&ESPNOW2buffer.buffer[eSPNOW2buffer.len],&theData[0],ln);
        eSPNOW2buffer.len +=  ln;
        break;
    }
}

void bufferSerial() {
    DBG("Buffering Serial.");
    memcpy(&SERIALbuffer.buffer[sERIALbuffer.len],&theData[0],ln);
    sERIALbuffer.len += ln;
    //UART_IF.println("SENDSERIAL:" + String(sERIALbuffer.len) + " ");
}

void bufferMQTT() {
    DBG("Buffering MQTT.");
    memcpy(&MQTTbuffer.buffer[mQTTbuffer.len],&theData[0],ln);
    mQTTbuffer.len += ln;
}

void bufferLoRa(uint8_t interface) {
  DBG("Buffering LoRa.");
  switch (interface) {
    case 0:
        memcpy(&LORAGbuffer.buffer[lORAGbuffer.len],&theData[0],ln);
        lORAGbuffer.len += ln;
      break;
    case 1:
        memcpy(&LORA1buffer.buffer[lORA1buffer.len],&theData[0],ln);
        lORA1buffer.len += ln;
        break;
    case 2:
        memcpy(&LORA2buffer.buffer[lORA2buffer.len],&theData[0],ln);
        lORA2buffer.len += ln;
        break;
  }
}


void espSend(uint8_t *mac,DataReading *buffer, uint16_t *len){

    DataReading thePacket[espnow_size];
    int j = 0;
    for (int i = 0; i < *len; i++) {
        if ( j > espnow_size) {
            j = 0;
            esp_now_send(mac, (uint8_t *) &thePacket, sizeof(thePacket));
        }
        thePacket[j] = buffer[i];
        j++;
    }
    esp_now_send(mac, (uint8_t *) &thePacket, j * sizeof(DataReading));
    *len = 0;
}

void releaseESPNOW(uint8_t interface) {
    DBG("Releasing ESP-NOW.");
    switch (interface) {
        case 0:
            espSend(broadcast_mac,ESPNOWGbuffer.buffer,&eSPNOWGbuffer.len);    
            break;
        case 1:
            espSend(ESPNOW1,ESPNOW1buffer.buffer,&eSPNOW1buffer.len);
            break;
        case 2:
            espSend(ESPNOW2,ESPNOW2buffer.buffer,&eSPNOW2buffer.len);
            break;
    }
}


void transmitLoRa(uint8_t* mac, DataReading * packet, uint8_t len) {
#ifdef USE_LORA    
  DBG("Transmitting LoRa.");

  uint8_t pkt[5 + (len * sizeof(DataReading))];
  memcpy(&pkt, mac, 3);
  memcpy(&pkt[3], &selfAddress[4], 2);
  memcpy(&pkt[5], packet, len * sizeof(DataReading));
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
#endif
}


void LoRaSend(uint8_t *mac,DataReading *buffer, uint16_t *len){

    DataReading thePacket[espnow_size];
    int j = 0;
    for (int i = 0; i < *len; i++) {
        if ( j > espnow_size) {
            j = 0;
            transmitLoRa(mac, thePacket, j);
        }
        thePacket[j] = buffer[i];
        j++;
    }
    transmitLoRa(broadcast_mac, thePacket, j);
    *len = 0;
}


void releaseLoRa(uint8_t interface) {
#ifdef USE_LORA
  DBG("Releasing LoRa.");

    switch (interface) {
    case 0:     
        LoRaSend(broadcast_mac,LORAGbuffer.buffer,&lORAGbuffer.len); 
        break;
    case 1:
        LoRaSend(LoRa1,LORA1buffer.buffer,&lORA1buffer.len); 
        break;
    case 2:
        LoRaSend(LoRa2,LORA2buffer.buffer,&lORA2buffer.len); 
        break;
    }
#endif
}

void releaseSerial() {
    DBG("Releasing Serial.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < sERIALbuffer.len; i++) {
        doc[i]["id"]   = SERIALbuffer.buffer[i].id;
        doc[i]["type"] = SERIALbuffer.buffer[i].t;
        doc[i]["data"] = SERIALbuffer.buffer[i].d;
    }
    serializeJson(doc, UART_IF);
    UART_IF.println();
    sERIALbuffer.len = 0;
}

void releaseMQTT() {
#ifdef USE_WIFI
    DBG("Releasing MQTT.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < mQTTbuffer.len; i++) {
        doc[i]["id"]   = MQTTbuffer.buffer[i].id;
        doc[i]["type"] = MQTTbuffer.buffer[i].t;
        doc[i]["data"] = MQTTbuffer.buffer[i].d;
    }
    String outgoingString;
    serializeJson(doc, outgoingString);
    client.publish(TOPIC_DATA, (char*) outgoingString.c_str());
    mQTTbuffer.len = 0;
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
            break;
        }
        DBG("Connecting MQTT.");
        delay(5000);
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
    esp_now_register_send_cb(ESP8266OnDataSent);
    esp_now_register_recv_cb(ESP8266OnDataRecv);

    // Register peers
    #ifdef ESPNOW1_PEER
    esp_now_add_peer(ESPNOW1, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
    #endif

    #ifdef ESPNOW2_PEER
    esp_now_add_peer(ESPNOW2, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
    #endif

#elif defined(ESP32)
    esp_wifi_set_mac(WIFI_IF_STA, &selfAddress[0]);
    
    if(esp_now_init() != ESP_OK) {
        DBG("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(ESP32OnDataSent);
    esp_now_register_recv_cb(ESP32OnDataRecv);

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

