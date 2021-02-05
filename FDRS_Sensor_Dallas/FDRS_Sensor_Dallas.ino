//  FARM DATA RELAY SYSTEM
//  
//  DALLAS DS18B20 SENSOR MODULE
//  
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a one-byte identifier.

#define SENSOR_ID 3
#define TERM_MAC 0x00 //Terminal MAC

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2


uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00};
int next_send = 0;

OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.

typedef struct DataReading {
  float t;
  float h;
  byte n;
} DataReading;

DataReading theData;


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void loadData() {
  sensors.requestTemperatures();
  theData.h = 0;
  theData.t = sensors.getTempFByIndex(0);
  theData.n = SENSOR_ID;
}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if (millis() > next_send) {
    loadData();
    esp_now_send(broadcastAddress, (uint8_t *) &theData, sizeof(theData));
    next_send = millis() + 15000;
  }
}
