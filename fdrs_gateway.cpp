#include "fdrs_gateway.h"

// #define ESP8266
#define ESP32

#define USE_WIFI

uint8_t newData = 0;
uint8_t ln;
DataReading theData[256];

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


