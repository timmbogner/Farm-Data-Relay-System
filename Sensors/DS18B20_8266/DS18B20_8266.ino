//  FARM DATA RELAY SYSTEM
//
//  DS18B20 SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type
//

#define TERM_MAC    0x00 //Terminal MAC
#define SLEEPYTIME  60   //Time to sleep in seconds
#define TEMP_ID     1    //Unique ID (0 - 255) for each data reading
#define ONE_WIRE_BUS 2   //Pin that the DS18B20 is connected to

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OneWire.h>
#include <DallasTemperature.h>

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};
int wait_time = 0;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

typedef struct DataPacket {
  uint8_t l;
  DataReading packet[30];

} DataPacket;


void setup() {
  Serial.begin(115200);
  sensors.begin();
  Serial.println("DS18B20 Initialized");
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
}

void loop() {
  if (millis() > wait_time) {
    wait_time = wait_time + SLEEPYTIME * 1000;
    loadData();
  }
}
void loadData() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0);
  DataReading Temp;
  Temp.id = TEMP_ID;
  Temp.t = 1;
  if (tempC = DEVICE_DISCONNECTED_C) Temp.d = -69.00;
  else Temp.d = tempC;
  DataPacket thePacket;
  thePacket.packet[0] = Temp;
  thePacket.l = 1;
  esp_now_send(broadcastAddress, (uint8_t *) &thePacket, sizeof(thePacket));

}
