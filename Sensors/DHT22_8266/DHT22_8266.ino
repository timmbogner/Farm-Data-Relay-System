//  FARM DATA RELAY SYSTEM
//
//  DHT SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each reading is assigned a two-byte identifier along with a one-byte sensor type
//

#define READING_ID     1    //Unique integer for each data reading
#define SLEEPYTIME     30   //Time to sleep in seconds
#define GTWY_MAC       0x00 //Gateway MAC
#define DHT_PIN        4


#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DHTesp.h"
#include "DataReading.h"


DHTesp dht;
uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, GTWY_MAC};
int wait_time = 0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}


void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("DHT Initialized");
  WiFi.mode(WIFI_STA);

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
  float h = dht.getHumidity();
  float t = dht.getTemperature();
  DataReading Temp;
  Temp.id = READING_ID;
  Temp.t = 1;
  DataReading Hum;
  Temp.id = READING_ID;
  Temp.t = 2;
  if (!(isnan(h) || isnan(t))) {
    Temp.d = -69.00;
    Hum.d = -4.20;
  } else {
    Temp.d = t;
    Hum.d = h;
  }
  DataPacket thePacket;
  thePacket.packet[0] = Temp;
  thePacket.packet[1] = Hum;

  esp_now_send(broadcastAddress, (uint8_t *) &thePacket, sizeof(thePacket));

}
