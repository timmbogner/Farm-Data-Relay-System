#if defined(ESP32) || defined(ESP8266)
#include <ArduinoOTA.h>

void begin_OTA() {
    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    //
    ArduinoOTA.setHostname("FDRSGW");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      }
      else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      //ESP_LOGI(TAG, "Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      DBG("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifndef USE_OLED  // Displaying on OLED slows down the download process
      DBG("Progress: " + String(progress / (total / 100)) + "%");
#endif
    });
    ArduinoOTA.onError([](ota_error_t error) {
      DBG("Error[" + String(error) + "]");
      if (error == OTA_AUTH_ERROR) { DBG("Auth Failed"); }
      else if (error == OTA_BEGIN_ERROR) { DBG("Begin Failed"); }
      else if (error == OTA_CONNECT_ERROR) { DBG("Connect Failed"); }
      else if (error == OTA_RECEIVE_ERROR) { DBG("Receive Failed"); }
      else if (error == OTA_END_ERROR) { DBG("End Failed"); }
    });
    
    ArduinoOTA.begin();
}

void handleOTA() {
    ArduinoOTA.handle();
}

#endif // defined(ESP32) || defined (ESP8266)