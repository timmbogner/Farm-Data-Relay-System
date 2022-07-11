// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// Modified by Timm Bogner for Farm Data Relay System -- Untested because I don't have a DHT sensor onhand. 

#include "fdrs_sensor_config.h"
#include <fdrs_sensor.h>
#include "DHT.h"

#define DHTPIN 2     // Digital pin connected to the DHT sensor

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  beginFDRS();
  DBG("DHTxx Sketch!");
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
  DBG("Failed to read from DHT sensor!");
    return;
  }
  loadFDRS(h, HUMIDITY_T);
  loadFDRS(t, TEMP_T);
  sendFDRS();
  sleepFDRS(10);  //Sleep time in seconds
}
