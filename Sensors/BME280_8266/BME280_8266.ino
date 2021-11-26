//  FARM DATA RELAY SYSTEM
//
//  BME280 SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each reading is assigned a two-byte identifier along with a one-byte sensor type
//  

#define READING_ID     63   //Unique integer for each data reading
#define SLEEPYTIME     30   //Time to sleep in seconds
#define GTWY_MAC       0x00 //Gateway MAC

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_BME280.h>
#include "DataReading.h"

Adafruit_BME280 bme;     //0x76
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
  WiFi.mode(WIFI_STA);
  Wire.begin();
  while (!bme.begin(0x76)) {
    delay(10);
  }
  Serial.println("bme INIT");
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if (millis() > wait_time) {
    wait_time = wait_time + SLEEPYTIME * 1000;
    loadData();
  }
}
void loadData() {
  float bme_temp = bme.readTemperature();
  float bme_pressure = (bme.readPressure() / 100.0F);
  //float bme_altitude = bme.readAltitude(1013.25);
  float bme_humidity = bme.readHumidity();
  Serial.println(bme_temp);
  DataReading Temp;
  Temp.d = bme_temp;
  Temp.id = READING_ID;
  Temp.t = 1;

  DataReading Hum;
  Hum.d = bme_humidity;
  Hum.id = READING_ID;
  Hum.t = 2;

  DataReading Pres;
  Pres.d = bme_pressure;
  Pres.id = READING_ID;
  Pres.t = 3;
  
  DataReading thePacket[3];
  thePacket[0] = Temp;
  thePacket[1] = Hum;
  thePacket[2] = Pres; 
  esp_now_send(broadcastAddress, (uint8_t *) &thePacket, sizeof(thePacket));

}
