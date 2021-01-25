//  FARM DATA RELAY SYSTEM
//  
//  GATEWAY MODULE
//  
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Setup instructions available in the "fdrs_config" file.

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "fdrs_config.h"

#define STASSID "user"
#define STAPSK  "pass"

const char* ssid     = STASSID;
const char* password = STAPSK;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

uint8_t prevAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, PREV_MAC};
uint8_t selfAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, UNIT_MAC};

char auth[] = "";

typedef struct DataReading {
  float t;
  float h;
  byte n;
} DataReading;

DataReading theData[6];
bool ledStatus = false;
bool newData = false;
bool failToggle = false;
unsigned long signalTimeout = 60000 * 15;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
    ledStatus = true;
  }
  else {
    Serial.println("Delivery fail");
    ledStatus = false;
  }
  digitalWrite(LED_BUILTIN, ledStatus);
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&theData, incomingData, sizeof(theData));
  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("Temp: ");
  Serial.println(theData[0].t);
  Serial.print("Humidity: ");
  Serial.println(theData[0].h);
  Serial.print("ID: ");
  Serial.println(theData[0].n);
  newData = true;

}
void handleBlynk() {
  signalTimeout = millis() + 60000 * 15;
  failToggle = false;
  newData = false;
  Blynk.run();
  Blynk.virtualWrite(V1, theData[0].t);
  Blynk.virtualWrite(V3, theData[1].t);
  Blynk.virtualWrite(V5, theData[2].t);
}
void failBlynk() {
  failToggle = true;
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int currentMonth = ptm->tm_mon + 1;
  int monthDay = ptm->tm_mday;
  String theOutput = String(currentMonth) + "." +  String(monthDay) + "." + timeClient.getFormattedTime();
  Serial.print(theOutput);
  Blynk.virtualWrite(V1, "Fail");
  Blynk.virtualWrite(V3, "Mode");
  Blynk.virtualWrite(V5, theOutput);
}
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println("Sola Gratia FDRS Gateway");
  Serial.print("Original MAC: ");
  Serial.println(WiFi.macAddress());
  wifi_set_macaddr(STATION_IF, selfAddress);
  Serial.println("New MAC:" + WiFi.macAddress());
  Serial.print("Previous device: ");
  Serial.println(PREV_MAC);
  Blynk.begin(auth, ssid, password);
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  esp_now_add_peer(prevAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  timeClient.begin();
  timeClient.setTimeOffset(-21600);

}

void loop() {
  Blynk.run();
  if (newData) handleBlynk();
  if ((!failToggle ) && ( millis() > signalTimeout)) failBlynk();

}
