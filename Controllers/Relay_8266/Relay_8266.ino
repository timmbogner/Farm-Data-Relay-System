//  FARM DATA RELAY SYSTEM
//
//  Electronic Relay Controller
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
//

#define CONTROLLER_ID 100    //Unique ID (0 - 255) for controller
#define UNIT_MAC      0x15  //Unit Address
#define TERM_MAC      0x00  //Terminal MAC
#define UPDATE_TIME   360   //Time in seconds between status update packets
#define COIL_PIN      5

#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};

typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

DataReading theCMD;
bool control_state = false;
bool connection_state = false;
bool newData = false;
unsigned long wait_time = 0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
    connection_state = true;
  }
  else {
    Serial.println("Delivery fail");
    connection_state = false;
  }
}

void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&theCMD, incomingData, sizeof(theCMD));
  if (theCMD.id == CONTROLLER_ID) {
    control_state = (bool)theCMD.d;
  }
  newData = true;
  connection_state = true;

}

void sayHello() {
  DataReading Packet;
  Packet.id = CONTROLLER_ID;
  Packet.t = 200;
  Packet.d = (float)control_state;
  esp_now_send(broadcastAddress, (uint8_t *) &Packet, sizeof(DataReading));
}

void setup() {
  pinMode(COIL_PIN, OUTPUT);
  digitalWrite(COIL_PIN, LOW);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_macaddr(STATION_IF, selfAddress);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  Serial.println("FARM DATA RELAY SYSTEM :: Irrigation Module");
  Serial.println("MAC:" + WiFi.macAddress());
}

void loop() {
  if (millis() > wait_time) {
    wait_time = wait_time + (UPDATE_TIME * 1000);
    sayHello();
  }
  if (!connection_state) control_state = false;
  if (control_state) {
    digitalWrite(COIL_PIN, HIGH);
  } else {
    digitalWrite(COIL_PIN, LOW);
  }
  if (newData) {
    Serial.println("DATA");
    newData = false;
  }
}
