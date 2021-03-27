//  FARM DATA RELAY SYSTEM
//
//  BME280 SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type
//  !!Untested on actual hardware!!

#define TERM_MAC    0x00 //Terminal MAC
#define SLEEPYTIME  10   //Time to sleep in seconds
#define TEMP_ID     0    //Unique ID (0 - 1023) for each data reading
#define HUM_ID      1
#define PRESS_ID    2

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bmp;     //0x77
uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};

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
  Wire.begin();
  uint8_t addr = 0x76;
  while (!bmp.begin()) {
    delay(10);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
void loadData() {
  float bme_temp = bmp.readTemperature();
  float bme_pressure = (bmp.readPressure() / 100.0F);
  //float bme_altitude = bmp.readAltitude(1013.25);
  float bme_humidity = bmp.readHumidity();
}
