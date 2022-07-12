
//  FARM DATA RELAY SYSTEM
//
//  Basic Sensor Example
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  An example of how to send data using "fdrs_sensor.h".
//

// compile error when defined here - why?
//#define USE_LORA
//#define USE_ESPNOW


#include "fdrs_sensor_config.h"
#include "fdrs_sensor.h"

#if defined(USE_LORA)
FDRSLoRa FDRS(GTWY_MAC,READING_ID,SPI_MISO,SPI_MOSI,SPI_SCK,LORA_SS,LORA_RST,LORA_DIO0,FDRS_BAND,FDRS_SF);
#elif defined(USE_ESPNOW)
FDRS_EspNow FDRS(GTWY_MAC, READING_ID); 
#endif

float data1;
float data2;

void setup() {
  FDRS.begin();
}
void loop() {
  data1 = readHum();
  FDRS.load(data1, HUMIDITY_T);
  data2 = readTemp();
  FDRS.load(data2, TEMP_T);
  FDRS.send();
  FDRS.sleep(10);  //Sleep time in seconds
}

float readTemp() {
  return 42.069;
}

float readHum() {
  return random(0,100);
}
