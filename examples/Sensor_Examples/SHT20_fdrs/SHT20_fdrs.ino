//  FARM DATA RELAY SYSTEM
//
//  SENSIRION SHT20 SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  If you are using the sensor for air monitoring, change SOIL_T to HUMIDITY_T.
//
#include "DFRobot_SHT20.h"
#include "fdrs_node_config.h"
#include <fdrs_node.h>

DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);


void setup() {
  beginFDRS();
   sht20.initSHT20();
}
void loop() {
  loadFDRS(sht20.readHumidity(), SOIL_T);
  loadFDRS(sht20.readTemperature(), TEMP_T);
  sendFDRS();
  sleepFDRS(300);  //Sleep time in seconds
}
