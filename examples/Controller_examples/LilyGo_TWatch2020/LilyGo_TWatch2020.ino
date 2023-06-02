//  FARM DATA RELAY SYSTEM
//
//  LilyGo T-Watch 2020 (v1) Example
//   This sketch retrieves the time from a nearby FDRS gateway using ESP-NOW.
//  
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA
//  FDRS Timekeeping was developed and contributed by Jeff Lehman (aviateur17)
//  
//  Gratitude to Lewis He and LilyGo for building and supporting the T-Watch.
//

#define UPTIME 3  // Seconds to remain on before sleeping
#define LILYGO_WATCH_2020_V1

#include <LilyGoWatch.h>
#include "fdrs_node_config.h"
#include <fdrs_node.h>

TTGOClass *ttgo;
TFT_eSPI *tft;
PCF8563_Class *rtc;
AXP20X_Class *power;

uint32_t interval = 0;

void fdrs_recv_cb(DataReading theData) {
}

void setup() {
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  rtc = ttgo->rtc;
  tft = ttgo->tft;
  power = ttgo->power;

  tft->setTextColor(TFT_RED, TFT_BLACK);
  //Draw voltage in top right corner
  tft->drawString(String(power->getBattVoltage() / 1000.0), 180, 0, 4);

  beginFDRS();
  uint32_t contact = pingFDRS(100);
  if (contact != UINT32_MAX) {
    // Draw ping response time (if any) in top left
    tft->drawString(String(contact), 0, 0, 4);
    addFDRS(fdrs_recv_cb);
    delay(100);
    rtc->syncToRtc();
  }

  char strftime_buf[64];
  time_t local = time(NULL) + (isDST ? dstOffset : stdOffset);
  localtime_r(&local, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%m-%d-%Y", &timeinfo);  // generate MM-DD-YYYY local time character array
  tft->setTextColor(TFT_BLUE, TFT_BLACK);
  tft->drawString(strftime_buf, 50, 200, 4);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  strftime(strftime_buf, sizeof(strftime_buf), "%I:%M:%S", &timeinfo);  // generate HH:MM:SS local time character array
  tft->drawString(strftime_buf, 5, 75, 7);
  delay(UPTIME * 1000);

  power->clearIRQ();
  ttgo->displaySleep();
  ttgo->powerOff();
  esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
  esp_deep_sleep_start();
}

void loop() {
}