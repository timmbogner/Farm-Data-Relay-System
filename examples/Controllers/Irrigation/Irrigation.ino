//  FARM DATA RELAY SYSTEM
//
//  Irrigation Controller
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
//
#define CONTROL_1    133        //Address for controller 1
#define CONTROL_2    134        //Address for controller 2
#define CONTROL_3    135        //Address for controller 3
#define CONTROL_4    136        //Address for controller 4
#define GTWY_MAC     0x00       //Gateway MAC
#define COIL_1       5          //Coil Pin 1
#define COIL_2       4          //Coil Pin 2
#define COIL_3       4          //Coil Pin 3
#define COIL_4       4          //Coil Pin 4

#include <FastLED.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#endif

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, GTWY_MAC};

typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

const uint16_t espnow_size = 250 / sizeof(DataReading);
DataReading theCommands[espnow_size];

int status_1 = 0;
int status_2 = 0;
int status_3 = 0;
int status_4 = 0;

bool newData = false;
int pkt_readings;
int wait_time = 0;

#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  memcpy(&theCommands, incomingData, len);
  pkt_readings = len / sizeof(DataReading);
  for (int i; i <= pkt_readings; i++) {        //Cycle through array of incoming DataReadings for any addressed to this device
    switch (theCommands[i].id) {
      case CONTROL_1:
        status_1 = (int)theCommands[i].d;
        Serial.println("D:" + String(theCommands[i].d));
        newData = true;
        break;
      case CONTROL_2:
        status_2 = (int)theCommands[i].d;
        Serial.println("D:" + String(theCommands[i].d));
        newData = true;
        break;
      case CONTROL_3:
        status_3 = (int)theCommands[i].d;
        Serial.println("D:" + String(theCommands[i].d));
        newData = true;
        break;
      case CONTROL_4:
        status_4 = (int)theCommands[i].d;
        Serial.println("D:" + String(theCommands[i].d));
        newData = true;
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
#if defined(ESP8266)
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
#elif defined(ESP32)
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
#endif
  pinMode(5, OUTPUT);
  digitalWrite(COIL_1, LOW);
  pinMode(COIL_2, OUTPUT);
  digitalWrite(COIL_2, LOW);
  pinMode(COIL_3, OUTPUT);
  digitalWrite(COIL_3, LOW);
  pinMode(COIL_4, OUTPUT);
  digitalWrite(COIL_4, LOW);
  
  Serial.println();
  Serial.println("FARM DATA RELAY SYSTEM :: Irrigation Module");

}
void updateCoils() {
  if (status_1) {
    digitalWrite(COIL_1, HIGH);
  } else {
    digitalWrite(COIL_1, LOW);
  }
  if (status_2) {
    digitalWrite(COIL_2, HIGH);
  } else {
    digitalWrite(COIL_2, LOW);
  }
  if (status_3) {
    digitalWrite(COIL_3, HIGH);
  } else {
    digitalWrite(COIL_3, LOW);
  }
  if (status_4) {
    digitalWrite(COIL_4, HIGH);
  } else {
    digitalWrite(COIL_4, LOW);
  }
}
void loop()
{
  if (newData) {
    newData = false;
    updateCoils();
  }
}
