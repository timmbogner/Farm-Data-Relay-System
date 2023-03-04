//  FARM DATA RELAY SYSTEM
//
//  MLX90614 INFRARED TEMPERATURE SENSOR MODULE
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.

#include "fdrs_node_config.h"
#include <Adafruit_MLX90614.h>
#include <fdrs_node.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  beginFDRS();
  delay(250);
  DBG("Adafruit MLX90614 test");
  if (!mlx.begin()) {
    DBG("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
}

void loop() {
  loadFDRS(mlx.readAmbientTempC(), TEMP_T);
  loadFDRS(mlx.readObjectTempC(), TEMP2_T);
  sendFDRS();
  sleepFDRS(60);  //Sleep time in seconds
}
