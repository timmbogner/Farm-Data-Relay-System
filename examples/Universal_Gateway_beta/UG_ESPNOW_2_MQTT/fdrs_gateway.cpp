#include "fdrs_gateway.h"

// #define ESP8266
// #define ESP32

// #define USE_WIFI

bool ESP_FDRSGateWay::is_init = false;
std::vector<Peer_t> ESP_FDRSGateWay::_peer_list;
std::vector<Peer_t> ESP_FDRSGateWay::_unknow_peer;

std::vector<DataReading_t> MQTT_FDRSGateWay::_buffer;
std::vector<DataReading_t> Serial_FDRSGateWay::_buffer;

// Set ESP-NOW send and receive callbacks for either ESP8266 or ESP32
void ESP_FDRSGateWay::OnDataRecv(uint8_t * mac, const uint8_t *incomingData, int len){

    DataReading_t data;
    uint32_t i = 0;
    uint32_t j = 0;
    
    uint8_t d = len / sizeof(DataReading_t);

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),mac,6) == 0){
            for(j = 0; j < d; j++){
                memcpy(&data,&incomingData[j*sizeof(DataReading_t)],sizeof(DataReading_t));
                _peer_list[i].buffer.push_back(data);
                memset(&data,0,sizeof(DataReading_t));
            }
            return;
        }  
    }

    for(uint32_t i = 0; i < _unknow_peer.size();i++){
        if(memcmp(_unknow_peer[i]._get_peer(),mac,6) == 0){
            for(j = 0; j < d; j++){
                memcpy(&data,&incomingData[j*sizeof(DataReading_t)],sizeof(DataReading_t));
                _unknow_peer[i].buffer.push_back(data);
                memset(&data,0,sizeof(DataReading_t));
            }
            return;
        }  
    }

    Peer_t peer;
    peer._set_peer(mac);
    _unknow_peer.push_back(peer);

    for(j = 0; j < d; j++){
        memcpy(&data,&incomingData[j*sizeof(DataReading_t)],sizeof(DataReading_t));
        _unknow_peer.back().buffer.push_back(data);
        memset(&data,0,sizeof(DataReading_t));
    }

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

FDRSGateWayBase::FDRSGateWayBase(){

}

FDRSGateWayBase::~FDRSGateWayBase(){
}

void FDRSGateWayBase::release(std::vector<DataReading_t> data,uint8_t *peer_mac){
    if(peer_mac == NULL){
        send(data);
    }
    forward(peer_mac ,data);
    
}

ESP_FDRSGateWay::ESP_FDRSGateWay(void)
{

    memset(_broadcast_mac,0xFF,6);
    memset(_inturnal_mac,0,6);

}

void ESP_FDRSGateWay::init(uint8_t inturnal_mac[5]){

    memcpy(_inturnal_mac,inturnal_mac,6);

#if defined(ESP8266)
    wifi_set_macaddr(STATION_IF, _inturnal_mac);
#endif

#if defined(ESP32)
    esp_wifi_set_mac(WIFI_IF_STA, &_inturnal_mac[0]);
#endif

    ESP_FDRSGateWay::setup();

#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
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

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),peer_mac,6) == 0){
            return;
        }  
    }

    list_peer(peer_mac);

    Peer_t peer;
    peer._set_peer(peer_mac);
    //esp_now_del_peer(NEWPEER);

    _peer_list.push_back(peer);

}

void ESP_FDRSGateWay::remove_peer(uint8_t peer_mac[6]){

    unlist_peer(peer_mac);

    if(_peer_list.size() == 0){
        return;
    }
    Peer_t peer;
    peer._set_peer(peer_mac);
    _peer_list.erase(std::find(_peer_list.begin(),_peer_list.end(),peer));

}

void ESP_FDRSGateWay::list_peer(uint8_t peer_mac[6]){

#if defined(ESP8266)
    esp_now_add_peer(peer_mac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#endif

#if defined(ESP32)
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
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

std::vector<DataReading_t> ESP_FDRSGateWay::get_peer_data(uint8_t *peer_mac){

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),peer_mac,6) == 0){
            return _peer_list[i].buffer;
        }  
    }
    return std::vector<DataReading_t>();
}

std::vector<DataReading_t> ESP_FDRSGateWay::get_unkown_peer_data(void){

    std::vector<DataReading_t> data;
    for(uint32_t i = 0; i < _unknow_peer.size(); i++){
        data.insert(data.end(),_unknow_peer[i].buffer.begin(),_unknow_peer[i].buffer.end());
    }

    return data;
}

void ESP_FDRSGateWay::flush(uint8_t *peer_mac){

    if(peer_mac == NULL){
        for(uint32_t i = 0; i < _unknow_peer.size(); i++){
            _unknow_peer[i].buffer.clear();
            return;
        }
    }

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),peer_mac,6) == 0){
            _peer_list[i].buffer.clear();
            return;
        }  
    }
    
}



void ESP_FDRSGateWay::send(std::vector<DataReading_t> data){

    const uint8_t espnow_size = 250 / sizeof(DataReading_t);

    uint32_t i = 0;
    for(i = 0; i < _unknow_peer.size(); i++){
        list_peer(_unknow_peer[i]._get_peer());
    }
    
    uint8_t d = data.size() / espnow_size;

    DataReading_t buffer1[d * sizeof(DataReading_t)];
    memset(buffer1,0,d * sizeof(DataReading_t));
    uint32_t j = 0;
    for(i = 0; i < data.size(); i++){

        buffer1[j++] = data[i];
        if(j >= d){
            esp_now_send(NULL, (uint8_t *) buffer1, d * sizeof(DataReading_t));
            
            memset(buffer1,0,d * sizeof(DataReading_t));
            j = 0;
            delay(10);
        }
        
    }

    if(j != 0){
        esp_now_send(NULL, (uint8_t *) buffer1, j * sizeof(DataReading_t));
    }

    for(i = 0; i < _unknow_peer.size(); i++){
        unlist_peer(_unknow_peer[i]._get_peer());
    }

    _unknow_peer.clear();
    
}

void ESP_FDRSGateWay::forward(uint8_t *peer_mac ,std::vector<DataReading_t> data){

    const uint8_t espnow_size = 250 / sizeof(DataReading_t);

    uint8_t d = data.size() / espnow_size;

    uint32_t i = 0;

    DataReading_t buffer1[d * sizeof(DataReading_t)];
    memset(buffer1,0,d * sizeof(DataReading_t));
    uint32_t j = 0;
    for(i = 0; i < data.size(); i++){

        buffer1[j++] = data[i];
        if(j >= d){
            esp_now_send(NULL, (uint8_t *) buffer1, d * sizeof(DataReading_t));
            
            memset(buffer1,0,d * sizeof(DataReading_t));
            j = 0;
            delay(10);
        }
        
    }

    if(j != 0){
        esp_now_send(NULL, (uint8_t *) buffer1, j * sizeof(DataReading_t));
    }

    esp_now_send(peer_mac, (uint8_t *) buffer1, d * sizeof(DataReading_t));
    
}

MQTT_FDRSGateWay::MQTT_FDRSGateWay(const char *ssid, const char *password, const char *server,int port):
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
        _buffer.push_back(data);
        memset(&data,0,sizeof(DataReading_t));
    }
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

void MQTT_FDRSGateWay::forward(uint8_t *peer_mac ,std::vector<DataReading_t> data){
    //does nothing. just here implement the pure virtule from the base class.
    send(data);
}

std::vector<DataReading_t> MQTT_FDRSGateWay::get_data(void){
    return _buffer;
}

void MQTT_FDRSGateWay::flush(uint8_t *peer_mac){
    _buffer.clear();
}

Serial_FDRSGateWay::Serial_FDRSGateWay(HardwareSerial *serial, uint32_t baud):
                    _serial(serial),
                    _baud(baud)
{
    
}

void Serial_FDRSGateWay::init(void){
    _serial->begin(_baud);
}

#if defined(ESP32)
void Serial_FDRSGateWay::init(int mode, int rx_pin, int tx_pin){
    _serial->begin(_baud,mode,rx_pin,tx_pin);
}
#endif

void Serial_FDRSGateWay::pull(void){
    //TDDO: This is blocking. Some method of escaping is required.
    // At the momment we are just hoping we get a \n
    String incomingString =  _serial->readStringUntil('\n');
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
    DataReading_t data;
    memset(&data,0,sizeof(DataReading_t));
    for (int i = 0; i < s; i++) {
        data.id = doc[i]["id"];
        data.type = doc[i]["type"];
        data.data = doc[i]["data"];
        _buffer.push_back(data);
        memset(&data,0,sizeof(DataReading_t));
    }
    DBG("Incoming Serial.");

}

std::vector<DataReading_t> Serial_FDRSGateWay::get_data(void){
    return _buffer;
}

void Serial_FDRSGateWay::get(void){
    while(_serial->available()){
        pull();
    }
}

void Serial_FDRSGateWay::send(std::vector<DataReading_t> data){

    DBG("Releasing Serial.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < data.size(); i++) {
        doc[i]["id"]   = data[i].id;
        doc[i]["type"] = data[i].type;
        doc[i]["data"] = data[i].data;
    }
    serializeJson(doc, *_serial);
    _serial->println();
}

void Serial_FDRSGateWay::forward(uint8_t *peer_mac ,std::vector<DataReading_t> data){
    //does nothing. just here implement the pure virtule from the base class.
}

void Serial_FDRSGateWay::flush(uint8_t *peer_mac){
    _buffer.clear();
}

LoRa_FDRSGateWay::LoRa_FDRSGateWay(uint8_t miso,uint8_t mosi,uint8_t sck, uint8_t ss,uint8_t rst,uint8_t dio0,double band,uint8_t sf):
                                _miso(miso),
                                _mosi(mosi),
                                _sck(sck),
                                _ss(ss),
                                _rst(rst),
                                _dio0(dio0),
                                _band(band),
                                _sf(sf)
{
    memset(_mac,0,6);
    _peer_list.clear();
}

void LoRa_FDRSGateWay::init(uint8_t mac[6]){

    memcpy(_mac,mac,6);

    DBG("Initializing LoRa!");
    #ifdef ESP32
        SPI.begin(_sck, _miso, _mosi);
    #endif
    LoRa.setPins(_ss, _rst, _dio0);
    if (!LoRa.begin(_band)) {
        DBG(" LoRa initialize failed");
        return;
    }
    LoRa.setSpreadingFactor(_sf);
    DBG(" LoRa initialized.");
}

void LoRa_FDRSGateWay::add_peer(uint8_t peer_mac[6]){

    uint32_t i = 0;

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),peer_mac,6) == 0){
            return;
        }  
    }

    Peer_t peer;
    peer._set_peer(peer_mac);
    _peer_list.push_back(peer);

}

void LoRa_FDRSGateWay::remove_peer(uint8_t peer_mac[6]){

    if(_peer_list.size() == 0){
        return;
    }
    Peer_t peer;
    peer._set_peer(peer_mac);
    _peer_list.erase(std::find(_peer_list.begin(),_peer_list.end(),peer));

}

void LoRa_FDRSGateWay::get(void){
    
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
    if (memcmp(&packet, &_mac[3], 3) != 0) { 
        return;    
    }

    uint8_t theData[packetSize - 5];

    memcpy(&incLORAMAC, &packet[3], 2);                  //Split off address portion of packet
    memcpy(&theData, &packet[5], packetSize - 5);        //Split off data portion of packet

    //Check if it is from a registered sender
    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(&_peer_list[i].peer[3],incLORAMAC,2) == 0){
            DataReading_t data;
            uint32_t i = 0;
            uint8_t d = packetSize / sizeof(DataReading_t);
            
            memset(&data,0,sizeof(DataReading_t));

            for(uint32_t j = 0; j < d; j++){
                memcpy(&data,&theData[j*sizeof(DataReading_t)],sizeof(DataReading_t));
                _peer_list[i].buffer.push_back(data);
                memset(&data,0,sizeof(DataReading_t));
            }

            return;
        }  
    }

    DBG("Incoming LoRa.");

}

std::vector<DataReading_t> LoRa_FDRSGateWay::get_peer_data(uint8_t *peer_mac){

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),peer_mac,6) == 0){
            return _peer_list[i].buffer;
        }  
    }
    return std::vector<DataReading_t>();
}

void LoRa_FDRSGateWay::send(std::vector<DataReading_t> data){

    const uint8_t espnow_size = 250 / sizeof(DataReading_t);

    uint32_t i = 0;
    uint8_t d = data.size() / espnow_size;

    DataReading_t buffer1[d];
    for(i = 0; i < d; i++){
        buffer1[i] = data[i];
    }

    transmit(buffer1, d * sizeof(DataReading_t));

}

void LoRa_FDRSGateWay::forward(uint8_t *peer_mac ,std::vector<DataReading_t> data){
    //TODO: add peer forwading.
}

void LoRa_FDRSGateWay::transmit(DataReading_t *packet, uint8_t len) {
    DBG("Transmitting LoRa.");
    uint8_t pkt[5 + (len * sizeof(DataReading_t))];
    memcpy(&pkt, _mac, 3);
    memcpy(&pkt[3], &_mac[4], 2);
    memcpy(&pkt[5], packet, len * sizeof(DataReading_t));
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pkt, sizeof(pkt));
    LoRa.endPacket();
}

void LoRa_FDRSGateWay::flush(uint8_t *peer_mac){

    if(peer_mac == NULL){
        return;
    }

    for(uint32_t i = 0; i < _peer_list.size();i++){
        if(memcmp(_peer_list[i]._get_peer(),peer_mac,6) == 0){
            _peer_list[i].buffer.clear();
            return;
        }  
    }
    
}

