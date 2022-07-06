//  FARM DATA RELAY SYSTEM
//
//  FastLED Controller
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Rest in Peace Dan Garcia, creator of FastLED.
//
#include <FastLED.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#endif


#define READING_ID    118        //Unique ID for controller
#define GTWY_MAC      0x00       //Gateway MAC

#define DATA_PIN    4
#define BAT_ADC     33
#define POWER_CTRL  5
#define NUM_LEDS    24


uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, GTWY_MAC};
CRGB leds[NUM_LEDS];

typedef struct DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

DataReading theCommands[31];
int the_color = 0;
int the_bright = 255;
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
    if (theCommands[i].id == READING_ID) {
      if (theCommands[i].t == 201) {           //Adjust color or brightness, depending on type.
        the_color = (int)theCommands[i].d;
        Serial.println("D:" + String(theCommands[i].d));
        newData = true;
      }
      if  (theCommands[i].t == 202) {
        the_bright = (int)theCommands[i].d;
        Serial.println("B:" + String(theCommands[i].d));
        newData = true;
      }
    }
  }
}

float readBattery()
{
  int vref = 1100;
  uint16_t volt = analogRead(BAT_ADC);
  Serial.println(volt);
  float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);
  return battery_voltage;
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
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
#endif

  Serial.println();
  Serial.println("FARM DATA RELAY SYSTEM :: Pretty Lantern FastLED Module");
  Serial.println("Thank You DAN GARCIA!");
  Serial.println("MAC: " + WiFi.macAddress());
  Serial.println("COLOR ID:  " + String(COLOR_ID));
  Serial.println("BRIGHT ID: " + String(BRIGHT_ID));
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.show();
  fill_solid(leds, NUM_LEDS, CRGB::Red); FastLED.show();
  delay(250);
  fill_solid(leds, NUM_LEDS, CRGB::Blue); FastLED.show();
  delay(250);
  fill_solid(leds, NUM_LEDS, CRGB::Green); FastLED.show();
  delay(250);
  fill_solid(leds, NUM_LEDS, CRGB::Black); FastLED.show();

}

void loop()
{
  if (newData) {
    newData = false;
    fill_solid(leds, NUM_LEDS, CHSV(the_color, 255, the_bright)); FastLED.show();
  }
  //  if (millis() > wait_time) {
  //    wait_time = wait_time + 30 * 1000;
  //    DataReading theVoltage;
  //    theVoltage.d = readBattery();
  //    theVoltage.id = BAT_ID;
  //    theVoltage.t = 50;
  //    esp_now_send(broadcastAddress, (uint8_t *) &theVoltage, sizeof(theVoltage));
  //
  //
  //  }
}
