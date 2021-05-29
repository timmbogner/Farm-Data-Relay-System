//  FARM DATA RELAY SYSTEM
//
//  BMP280 SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type
//

#define TERM_MAC    0x00 //Terminal MAC
#define SLEEPYTIME  15   //Time to sleep in seconds
#define TEMP_ID     1    //Unique ID (0 - 255) for each data reading


#define BASE_LAT  40.10766122891675
#define BASE_LON -88.21271402255591
#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <TinyGPS++.h>
#include "DataReading.h"

static const int RXPin = D5, TXPin = D6;
SoftwareSerial ss(RXPin, TXPin);
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
unsigned int delay_time = 0;

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};
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

void sendPing() {
  DataReading myLat;
  myLat.d = gps.location.lat();
  myLat.t = 20;
  myLat.id = 0;
  Serial.println(gps.location.lat());
  DataReading myLon;
  myLon.d = gps.location.lng();
  myLon.t = 21;
  myLon.id = 1;

  DataReading myDist;
  myDist.d = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), BASE_LAT, BASE_LON);
  myDist.t = 23;
  myDist.id = 2;
  DataReading myData[3];
  myData[0] = myLat;
  myData[1] = myLon;
  myData[2] = myDist;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

void setup() {
    ss.begin(GPSBaud);

  Serial.begin(9600);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  Serial.println("GPS init");
}

void loop() {
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
//      sendPing();
    }
  if (millis() > delay_time) {
    delay_time = millis() + (SLEEPYTIME * 1000);
    sendPing();
  }
}
