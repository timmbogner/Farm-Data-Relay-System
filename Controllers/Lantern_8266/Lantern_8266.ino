//  FARM DATA RELAY SYSTEM
//
//  Pretty Lantern Controller
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//
//

#define READING_ID    72    //Unique ID (0 - 255) for controller
#define GTWY_MAC      0x00  //Gateway MAC

#define REDPIN   12
#define GREENPIN 13
#define BLUEPIN  15

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, GTWY_MAC};

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
  }
  else {
    Serial.println("Delivery fail"); 
  }
}

void OnDataRecv(uint8_t* mac, uint8_t *incomingData, uint8_t len) {
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

void colorBars()
{
  showAnalogRGB( CRGB::Red );   delay(500);
  showAnalogRGB( CRGB::Green ); delay(500);
  showAnalogRGB( CRGB::Blue );  delay(500);
  showAnalogRGB( CRGB::Black ); delay(500);
}

void loop()
{

  if (newData) {
    newData = false;
    showAnalogRGB(CHSV(the_color, 255, the_bright));

  }

}


void setup() {
  pinMode(REDPIN,   OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN,  OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
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
