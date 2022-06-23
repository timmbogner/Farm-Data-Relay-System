#include "fdrs_gateway.h"

// #define ESP8266
// #define ESP32

// #define USE_WIFI

std::vector<DataReading_t> FDRSGateWayBase::_data;
std::vector<FDRSGateWayBase*> FDRSGateWayBase::_object_list;

bool ESP_FDRSGateWay::is_init = false;
std::vector<ESP_Peer_t> ESP_FDRSGateWay::peer_list;
std::vector<ESP_Peer_t> ESP_FDRSGateWay::unknow_peer;

bool MQTT_FDRSGateWay::is_init = false;

uint8_t newData = 0;
uint8_t ln = 0;
DataReading_t theData[256];

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

void ESP_FDRSGateWay::OnDataRecv(uint8_t * mac, const uint8_t *incomingData, int len){


    DataReading_t data;

    //memcpy(&data, incomingData, sizeof(theData));

    uint32_t i = 0;
    
    
    uint8_t d = len / sizeof(DataReading_t);

    for(i = 0; i < d; i++){
        memcpy(&data,&incomingData[i*sizeof(DataReading_t)],sizeof(DataReading_t));
        FDRSGateWayBase::add_data(&data);
        memset(&data,0,sizeof(DataReading_t));
    }

    for(uint32_t i = 0; i < peer_list.size();i++){
        if(memcmp(peer_list[i]._data(),mac,6) == 0){
            return;
        }  
    }

    ESP_Peer_t peer;
    peer._copy(mac);
    unknow_peer.push_back(peer);
}

#if defined(ESP8266)
void ESP8266OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {

}

void ESP8266OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
    ESP_FDRSGateWay::OnDataRecv((uint8_t*)mac,*(const uint8_t *)incomingData,len);
}
#endif

#if defined(ESP32)
void ESP32OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

}

void ESP32OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    ESP_FDRSGateWay::OnDataRecv((uint8_t*)mac,incomingData,len);
}
#endif



// void getLoRa() {
// #ifdef USE_LORA
//     int packetSize = LoRa.parsePacket();
//     if (packetSize== 0) {
//         return;
//     }
//     uint8_t packet[packetSize];
//     uint8_t incLORAMAC[2];
//     LoRa.readBytes((uint8_t *)&packet, packetSize);
//     //    for (int i = 0; i < packetSize; i++) {
//     //      UART_IF.println(packet[i], HEX);
//     //    }

//     //Check if addressed to this device
//     if (memcmp(&packet, &selfAddress[3], 3) != 0) { 
//         return;    
//     }

//     memcpy(&incLORAMAC, &packet[3], 2);                  //Split off address portion of packet
//     memcpy(&theData, &packet[5], packetSize - 5);        //Split off data portion of packet

//     //Check if it is from a registered sender
//     if(memcmp(&incLORAMAC, &LoRa1, 2) == 0){ 
//         newData = 7;
//     }     
//     else if(memcmp(&incLORAMAC, &LoRa2, 2) == 0){
//         newData = 8;
//     }

//     newData = 6;
//     ln = (packetSize - 5) / sizeof(DataReading_t);
    
//     DBG("Incoming LoRa.");
// #endif
// }

// void transmitLoRa(uint8_t* mac, DataReading_t * packet, uint8_t len) {
// #ifdef USE_LORA    
//   DBG("Transmitting LoRa.");

//   uint8_t pkt[5 + (len * sizeof(DataReading_t))];
//   memcpy(&pkt, mac, 3);
//   memcpy(&pkt[3], &selfAddress[4], 2);
//   memcpy(&pkt[5], packet, len * sizeof(DataReading_t));
//   LoRa.beginPacket();
//   LoRa.write((uint8_t*)&pkt, sizeof(pkt));
//   LoRa.endPacket();
// #endif
// }

// void LoRaSend(uint8_t *mac,DataReading_t *buffer, uint16_t *len){

//     DataReading_t thePacket[espnow_size];
//     int j = 0;
//     for (int i = 0; i < *len; i++) {
//         if ( j > espnow_size) {
//             j = 0;
//             transmitLoRa(mac, thePacket, j);
//         }
//         thePacket[j] = buffer[i];
//         j++;
//     }
//     transmitLoRa(broadcast_mac, thePacket, j);
//     *len = 0;
// }

// void releaseLoRa(uint8_t interface) {
// #ifdef USE_LORA
//   DBG("Releasing LoRa.");

//     switch (interface) {
//     case 0:     
//         LoRaSend(broadcast_mac,LORAGbuffer.buffer,&LORAGbuffer.len); 
//         break;
//     case 1:
//         LoRaSend(LoRa1,LORA1buffer.buffer,&LORA1buffer.len); 
//         break;
//     case 2:
//         LoRaSend(LoRa2,LORA2buffer.buffer,&LORA2buffer.len); 
//         break;
//     }
// #endif
// }

void bufferLoRa(uint8_t interface) {
  DBG("Buffering LoRa.");
  switch (interface) {
    case 0:
        memcpy(&LORAGbuffer.buffer[LORAGbuffer.len],&theData[0],ln);
        LORAGbuffer.len += ln;
      break;
    case 1:
        memcpy(&LORA1buffer.buffer[LORA1buffer.len],&theData[0],ln);
        LORA1buffer.len += ln;
        break;
    case 2:
        memcpy(&LORA2buffer.buffer[LORA2buffer.len],&theData[0],ln);
        LORA2buffer.len += ln;
        break;
  }
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
        theData[i].type = doc[i]["type"];
        theData[i].data = doc[i]["data"];
    }
    ln = s;
    newData = 4;
    DBG("Incoming Serial.");
}

void sendSerial() {
    DBG("Sending Serial.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < ln; i++) {
        doc[i]["id"]   = theData[i].id;
        doc[i]["type"] = theData[i].type;
        doc[i]["data"] = theData[i].data;
    }
    serializeJson(doc, UART_IF);
    UART_IF.println();

    #ifndef ESP8266
    serializeJson(doc, Serial);
    Serial.println();
    #endif
}

void bufferSerial() {
    DBG("Buffering Serial.");
    memcpy(&SERIALbuffer.buffer[SERIALbuffer.len],&theData[0],ln);
    SERIALbuffer.len += ln;
    //UART_IF.println("SENDSERIAL:" + String(SERIALbuffer.len) + " ");
}

void releaseSerial() {
    DBG("Releasing Serial.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < SERIALbuffer.len; i++) {
        doc[i]["id"]   = SERIALbuffer.buffer[i].id;
        doc[i]["type"] = SERIALbuffer.buffer[i].type;
        doc[i]["data"] = SERIALbuffer.buffer[i].data;
    }
    serializeJson(doc, UART_IF);
    UART_IF.println();
    SERIALbuffer.len = 0;
}

FDRSGateWayBase::FDRSGateWayBase(uint32_t send_delay): _send_delay(send_delay){
    _object_list.push_back(this);
}

FDRSGateWayBase::~FDRSGateWayBase(){
    if(_object_list.size() == 0){
        return;
    }
    _object_list.erase(std::find(_object_list.begin(),_object_list.end(),this));
}

void FDRSGateWayBase::release(void){

    for(int i =0; i < _object_list.size();i++){
        _object_list[i]->send(_data);
    }
    _data.clear();

}

void FDRSGateWayBase::add_data(DataReading_t *data){
    _data.push_back(*data);
}

ESP_FDRSGateWay::ESP_FDRSGateWay(uint8_t broadcast_mac[6],uint8_t inturnal_mac[5], uint32_t send_delay) : 
                                FDRSGateWayBase(send_delay)
{

    memcpy(_broadcast_mac,broadcast_mac,6);
    memcpy(_inturnal_mac,inturnal_mac,6);

}

void ESP_FDRSGateWay::init(void){

#if defined(ESP8266)
    wifi_set_macaddr(STATION_IF, _inturnal_mac);
#endif

#if defined(ESP32)
    esp_wifi_set_mac(WIFI_IF_STA, &_inturnal_mac[0]);
#endif

    ESP_FDRSGateWay::setup();

#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    // Register first peer
    memcpy(peerInfo.peer_addr, _broadcast_mac, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        DBG("Failed to add peer bcast");
        return;
    }
#endif

}

void ESP_FDRSGateWay::setup(void){
    if(is_init){
        return;
    }
    is_init = true;
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

#if defined(ESP8266)
    
    if (esp_now_init() != 0) {
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb(ESP8266OnDataSent);
    esp_now_register_recv_cb(ESP8266OnDataRecv);
#endif

#if defined(ESP32)
    
    
    if(esp_now_init() != ESP_OK) {
        DBG("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(ESP32OnDataSent);
    esp_now_register_recv_cb(ESP32OnDataRecv);

#endif

    DBG("ESP-NOW Initialized.");

}

void ESP_FDRSGateWay::add_peer(uint8_t peer_mac[6]){

    uint32_t i = 0;

    for(uint32_t i = 0; i < peer_list.size();i++){
        if(memcmp(peer_list[i]._data(),peer_mac,6) == 0){
            return;
        }  
    }

    list_peer(peer_mac);

    ESP_Peer_t peer;
    peer._copy(peer_mac);
    //esp_now_del_peer(NEWPEER);

    peer_list.push_back(peer);

}

void ESP_FDRSGateWay::remove_peer(uint8_t peer_mac[6]){

    unlist_peer(peer_mac);

    if(peer_list.size() == 0){
        return;
    }
    ESP_Peer_t peer;
    peer._copy(peer_mac);
    //peer_list.erase(std::find(peer_list.begin(),peer_list.end(),peer));

}

void ESP_FDRSGateWay::list_peer(uint8_t peer_mac[6]){

#if defined(ESP8266)
    esp_now_add_peer(peer_mac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#endif

#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, peer_mac, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        DBG("Failed to add peer 1");
        return;
    }
#endif
    
}

void ESP_FDRSGateWay::unlist_peer(uint8_t peer_mac[6]){

    esp_now_del_peer(peer_mac);
    
}

void ESP_FDRSGateWay::send(std::vector<DataReading_t> data){

    const uint8_t espnow_size = 250 / sizeof(DataReading_t);

    uint32_t i = 0;
    for(i = 0; i < unknow_peer.size(); i++){
        list_peer(unknow_peer[i]._data());
    }
    
    uint8_t d = data.size() / espnow_size;
    uint8_t r = data.size() % espnow_size;

    DataReading_t buffer1[d];
    for(i = 0; i < d; i++){
        buffer1[i] = data[i];
    }

    esp_now_send(NULL, (uint8_t *) buffer1, d * sizeof(DataReading_t));

    for(i = 0; i < r; i++){
        buffer1[i] = data[i + d];
    }

    esp_now_send(NULL, (uint8_t *) buffer1, r * sizeof(DataReading_t));

    

    for(i = 0; i < unknow_peer.size(); i++){
        unlist_peer(unknow_peer[i]._data());
    }

    unknow_peer.clear();
    
}

MQTT_FDRSGateWay::MQTT_FDRSGateWay(uint32_t send_delay, const char *ssid, const char *password, const char *server,int port):
                                    FDRSGateWayBase(send_delay),
                                    _ssid(ssid),
                                    _password(password),
                                    _server(server),
                                    _port(port)
{

    _client = new PubSubClient(espClient);

}

MQTT_FDRSGateWay::~MQTT_FDRSGateWay(void){
    delete _client;
}

void MQTT_FDRSGateWay::mqtt_callback(char* topic, byte * message, unsigned int length){

    //No point in reading topics that are not data.
    if(strcmp(TOPIC_DATA,topic) != 0){
        return;
    }

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
    DataReading_t data;
    memset(&data,0,sizeof(DataReading_t));
    for (int i = 0; i < s; i++) {
        data.id = doc[i]["id"];
        data.type = doc[i]["type"];
        data.data = doc[i]["data"];
        FDRSGateWayBase::add_data(&data);
        memset(&data,0,sizeof(DataReading_t));
    }
    ln = s;
    newData = 5;
    DBG("Incoming MQTT.");
    
}

void MQTT_FDRSGateWay::init(void){
    delay(10);
    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        DBG("Connecting to WiFi... ");
        DBG(_ssid);

        delay(500);
    }
    DBG("WiFi Connected");
    _client->setServer(_server, _port);
    if (!_client->connected()) {
        DBG("Connecting MQTT...");
        reconnect();
    }
    DBG("MQTT Connected");
    _client->setCallback(MQTT_FDRSGateWay::mqtt_callback);

    _client->publish(TOPIC_STATUS, "FDRS initialized");
}

void MQTT_FDRSGateWay::reconnect() {
    // Loop until reconnected
    while (!_client->connected()) {
    // Attempt to connect
        if (_client->connect("FDRS_GATEWAY")) {
            // Subscribe
            _client->subscribe(TOPIC_COMMAND);
            break;
        }
        DBG("Connecting MQTT.");
        delay(5000);
    }
}

void MQTT_FDRSGateWay::send(std::vector<DataReading_t> data) {

    DBG("Releasing MQTT.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < data.size(); i++) {
        doc[i]["id"]   = data[i].id;
        doc[i]["type"] = data[i].type;
        doc[i]["data"] = data[i].data;
    }
    String outgoingString;
    serializeJson(doc, outgoingString);
    _client->publish(TOPIC_DATA, (char*) outgoingString.c_str());

    reconnect();
    _client->loop();
    
}