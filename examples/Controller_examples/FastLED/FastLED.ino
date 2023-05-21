//  FARM DATA RELAY SYSTEM
//
//  FastLED Lantern
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  Rest in Peace, Daniel Garcia. Thank you for bringing so much light to the world!

//  Type
//   1: Red
//   2: Green
//   3: Blue
//   4: Hue
//   5: Saturation
//   6: Brightness
//
//

#define USE_PWM    //If using an RGB LED
// #define PIN_R  18   //ESP32 WeMos
// #define PIN_G  19
// #define PIN_B  23

#define PIN_R  14  //8266 WeMos
#define PIN_G  12
#define PIN_B  13

#define PIN_DATA 4  // If using a NeoPixel
#define NUM_LEDS 24  // Number of physical LEDs.

#include <FastLED.h>
#include "fdrs_node_config.h"
#include <fdrs_node.h>

CRGB rgb_color = CRGB::Black;
CHSV hsv_color(0, 255, 255);
bool hsv_mode = false;
bool new_data = false;

CRGB leds[NUM_LEDS];

void fdrs_recv_cb(DataReading theData) {

  new_data = true;
  int id = (int)theData.id;
  uint8_t type = (uint8_t)theData.t;
  uint8_t data = (uint8_t)theData.d;
  switch (type) {
    case 1:
      rgb_color.red = data;
      hsv_mode = false;
      break;
    case 2:
      rgb_color.green = data;
      hsv_mode = false;
      break;
    case 3:
      rgb_color.blue = data;
      hsv_mode = false;
      break;
    case 4:
      hsv_color.hue = data;
      hsv_mode = true;
      break;
    case 5:
      hsv_color.sat = data;
      hsv_mode = true;
      break;
    case 6:
      hsv_color.val = data;
      hsv_mode = true;
      break;
  }
}
void setup_pwm() {
#ifdef ESP8266
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
#endif
#ifdef ESP32
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(PIN_R, 0);
  ledcAttachPin(PIN_G, 1);
  ledcAttachPin(PIN_B, 2);
#endif
}

void set_color(CRGB new_color) {
#ifdef USE_PWM
#ifdef ESP8266
  analogWrite(PIN_R, new_color.r);
  analogWrite(PIN_G, new_color.g);
  analogWrite(PIN_B, new_color.b);
#elif ESP32
  ledcWrite(0, new_color.r);
  ledcWrite(1, new_color.g);
  ledcWrite(2, new_color.b);
#endif
#else
  fill_solid(leds, NUM_LEDS, new_color);
  FastLED.show();
#endif  // USE_PWM
}

void color_bars() {
  set_color(CRGB::Red);
  delay(250);
  set_color(CRGB::Green);
  delay(250);
  set_color(CRGB::Blue);
  delay(250);
  set_color(CRGB::Black);
}

void setup() {
#ifdef USE_PWM
  setup_pwm();
#else
  FastLED.addLeds<WS2812B, PIN_DATA, GRB>(leds, NUM_LEDS);
#endif
  color_bars();

  beginFDRS();
  addFDRS(1000, fdrs_recv_cb);
  subscribeFDRS(READING_ID);
}
void loop() {
  loopFDRS();
  if (new_data) {
    new_data = false;
    if (hsv_mode) {
      set_color(hsv_color);
    } else {
      set_color(rgb_color);
    }
  }
}