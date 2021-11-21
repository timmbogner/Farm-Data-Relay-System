//  FARM DATA RELAY SYSTEM
//
//  Pretty Lantern Controller
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
//

#define CONTROLLER_ID 72    //Unique ID (0 - 255) for controller
#define UNIT_MAC      0x15  //Unit Address
#define TERM_MAC      0x00  //Terminal MAC
#define UPDATE_TIME   360   //Time in seconds between status update packets
#define REDPIN   12
#define GREENPIN 13
#define BLUEPIN  15

#include <FastLED.h>
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
int the_color = 0;
bool connection_state = false;
bool newData = false;
unsigned long wait_time = 0;
void showAnalogRGB( const CRGB& rgb)
{
  analogWrite(REDPIN,   rgb.r );
  analogWrite(GREENPIN, rgb.g );
  analogWrite(BLUEPIN,  rgb.b );
}

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
  Serial.println("recv");

  if (theCMD.id == CONTROLLER_ID) {
    the_color = (int)theCMD.d;
  }
  newData = true;
  connection_state = true;


}

void sayHello() {
  DataReading Packet;
  Packet.id = CONTROLLER_ID;
  Packet.t = 200;
  Packet.d = (float)the_color;
  esp_now_send(broadcastAddress, (uint8_t *) &Packet, sizeof(DataReading));
}
void colorBars()
{
  showAnalogRGB( CRGB::Red );   delay(500);
  showAnalogRGB( CRGB::Green ); delay(500);
  showAnalogRGB( CRGB::Blue );  delay(500);
  showAnalogRGB( CRGB::Black ); delay(500);
}

void loop()
{

  if (millis() > wait_time) {
    wait_time = wait_time + (UPDATE_TIME * 1000);
    sayHello();
  }
  if (newData) {
    Serial.println("DATA");
    newData = false;
    showAnalogRGB(CHSV(the_color, 255, 255));
  }
}



void setup() {
  pinMode(REDPIN,   OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN,  OUTPUT);
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
  Serial.println("FARM DATA RELAY SYSTEM :: Pretty Lantern Module");
  Serial.println("MAC:" + WiFi.macAddress());
  colorBars();
}
