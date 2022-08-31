//  FARM DATA RELAY SYSTEM
//
//  ESP-NOW Sensor Example
//
//  Developed by Timm Bogner (timmbogner@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  An example of how to send data via ESP-NOW using FDRS.
//

#include "fdrs_sensor_config.h"
#include <fdrs_node.h>

void fdrs_recv_cb(DataReading theData) {
  DBG("ID: " + String(theData.id));
  DBG("Type: " + String(theData.t));
  DBG("Data: " + String(theData.d));
}

void setup() {
  beginFDRS();
  //pingFDRS(1000);
  addFDRS(1000, fdrs_recv_cb);
  subscribeFDRS(READING_ID);
}
void loop() {
  //    data1 = readHum();
  //    loadFDRS(data1, HUMIDITY_T);
  //    data2 = readTemp();
  //    loadFDRS(data2, TEMP_T);
  //    sendFDRS();
  //    sleepFDRS(10);  //Sleep time in seconds
}
