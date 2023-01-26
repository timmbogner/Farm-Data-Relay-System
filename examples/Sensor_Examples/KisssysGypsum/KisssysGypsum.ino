//  FARM DATA RELAY SYSTEM
//
//  Gypsum-based Soil Moisture Sensor
//
//  Uses a Ezsbc.com Dev board, a Kisssys moisture sensor board, and a DS3231 RTC.
//  Deep sleep current is less than 20µA.
//  https://www.printables.com/model/176752-gypson-water-sensor

#define DEBUG
#define CREDENTIALS

#include "fdrs_node_config.h"
#include <fdrs_node.h>
#include <RTClib.h>
RTC_DS3231 rtc;

#define BUTTON_PIN_BITMASK 0x100000000 // 2^32 in hex the pin that is connected to SQW
#define CLOCK_INTERRUPT_PIN 32  // yep same pin
//#define SupplyPin 33            // power to DS3231
#define BatteryReadPin 35        // read battey voltage on this pin


const int FreqIn1 = 18;   // gpio32
const int FreqPower = 19;
const int RTCPower = 25;
volatile uint16_t Freq1 = 0;
static uint16_t FreqOut1;

void SensorInt1() {
  // If the pin is Rising, increment counter
  Freq1++;
};



void setup() {
  beginFDRS();
  DBG(__FILE__);
  pinMode(FreqIn1, INPUT);
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP); // On deep sleep the pin needs and external 330k pullup
  pinMode(RTCPower, OUTPUT);                 // for the SQW pin to stay high when RTC power is removed
  digitalWrite(RTCPower, HIGH);              // DS3231 needs the SQW pullup removed on the board to
  pinMode(FreqPower, OUTPUT);                // lower the deepsleep current by 60ua's
  digitalWrite(FreqPower, HIGH);

  Freq1 = 0;
  delay(50);

  Wire.begin(13, 14);    // moved from 21 22 normal I2C pins because it allows me to set these pins to input later
  if (!rtc.begin()) {
    DBG("Couldn't find RTC!");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    // this will adjust to the date and time at compilation
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.disable32K();    // Don't use this so we shut it down on the board
  // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
  // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // stop oscillating signals at SQW Pin
  // otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  // turn off alarm 2 (in case it isn't off already)
  // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);


  // for one second we count the pulses and get our moisture reading in pps
  attachInterrupt(FreqIn1, SensorInt1, RISING);
  delay(1000);       // delay in ms is 1 seconds-- not the most accurate way to do this but more than accurate enough for our low frequency and 10 second read
  FreqOut1 = Freq1;       // our frequency * 10
  detachInterrupt(FreqIn1);   // only reading once so turn off the interrupt


  if (rtc.alarmFired(1)) {
    rtc.clearAlarm(1);
    Serial.println("Alarm cleared");
  }

  float supply = analogRead(BatteryReadPin) * .001833;
  DBG("WaterSensor 1 = " + String(FreqOut1));
  DBG("Supply Voltage = " + String(supply));

      loadFDRS(FreqOut1, SOIL_T);
      loadFDRS(supply, VOLTAGE_T);
      sendFDRS();
      // Lowers SQW int pin32 every 10 seconds
      rtc.setAlarm1(DateTime(0, 0, 0, 00, 00, 10), DS3231_A1_Second); //DateTime (year,month,day,hour,min,sec)

      esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);

      char date[10] = "hh:mm:ss";
      rtc.now().toString(date);
      Serial.println(date);     // Print the time
      Serial.println("Going to sleep soon");

      // prepare for low current shutdown
      digitalWrite(FreqPower, LOW);
      digitalWrite(RTCPower, LOW);
      pinMode(13, INPUT);
      pinMode(14, INPUT);

      esp_deep_sleep_start();

}

void loop() {
  // nuttin honey
}
