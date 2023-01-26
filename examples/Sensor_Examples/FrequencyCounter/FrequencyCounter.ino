//  FARM DATA RELAY SYSTEM
//
//  Gypsum-based Soil Moisture Sensor
//
//  Uses a Ezsbc.com Dev board, a Kisssys moisture sensor board, and a DS3231 RTC.
//  Deep sleep current is less than 20µA.
//  https://www.printables.com/model/176752-gypson-water-sensor


#include "fdrs_node_config.h"
#include <fdrs_node.h>


#define BUTTON_PIN_BITMASK 0x100000000 // 2^32 in hex the pin that is connected to SQW
#define CLOCK_INTERRUPT_PIN 32  // yep same pin


const int FreqIn1 = 5;
volatile uint16_t Freq1 = 0;
static uint16_t FreqOut1;

ICACHE_RAM_ATTR void SensorInt1() {
  // If the pin is Rising, increment counter
  Freq1++;
};


void readFrequency() {
  // for one second we count the pulses and get our moisture reading in pps
  attachInterrupt(FreqIn1, SensorInt1, RISING);
  delay(1000);       // delay in ms is 1 seconds-- not the most accurate way to do this but more than accurate enough for our low frequency and 10 second read
  FreqOut1 = Freq1;       // our frequency * 10
  detachInterrupt(FreqIn1);   // only reading once so turn off the interrupt

}
void setup() {
  beginFDRS();
  pinMode(FreqIn1, INPUT);
  Freq1 = 0;
  delay(50);
  readFrequency();
  //DBG("Frequency = " + String(FreqOut1));
  loadFDRS(FreqOut1, SOIL_T);
  sendFDRS();
  sleepFDRS(1800);  //Sleep time in seconds

}

void loop() {
  // nuttin honey
}
