//  FARM DATA RELAY SYSTEM
//
//  LILYGO HIGROW SENSOR MODULE
//


#define I2C_SDA             25
#define I2C_SCL             26
#define DHT12_PIN           16
#define BAT_ADC             33
#define SALT_PIN            34
#define SOIL_PIN            32
#define BOOT_PIN            0
#define USER_BUTTON         35
#define DS18B20_PIN         21

#include "fdrs_sensor_config.h"
#include <fdrs_sensor.h>
#include <BH1750.h>
#include <Adafruit_BME280.h>

BH1750 lightMeter(0x23); //0x23
Adafruit_BME280 bmp;     //0x77
RTC_DATA_ATTR int the_count = 0;

void setup() {

  //Init Sensors
  Wire.begin(I2C_SDA, I2C_SCL);
  while (!bmp.begin()) {
    //Serial.println("bmp");
    delay(10);
  }

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

}

void loadData() {
  float s_battery = readBattery();
  float bme_temp = bmp.readTemperature();
  float bme_pressure = (bmp.readPressure() / 100.0F);
  //float bme_altitude = bmp.readAltitude(1013.25);
  float bme_humidity = bmp.readHumidity();
  float s_soil = readSoil();
  float s_salt = readSalt();
  while (! lightMeter.measurementReady()) {
    delay(10);
  }
  float lux = lightMeter.readLightLevel();
  the_count++;

  Serial.println();
  Serial.println("Temp: " + String(bme_temp));
  Serial.println("Humidity: " + String(bme_humidity));
  Serial.println("Light: " + String(lux));
  Serial.println("Pressure: " + String(bme_pressure));
  Serial.println("Salt: " + String(s_salt));
  Serial.println("Soil: " + String(s_soil));
  Serial.println("Voltage: " + String(s_battery));
  Serial.println("Count: " + String(the_count));

  loadFDRS(bme_temp, TEMP_T);

  loadFDRS(bme_humidity, HUMIDITY_T);


  loadFDRS(lux, LIGHT_T);
  loadFDRS(bme_pressure, PRESSURE_T);
  loadFDRS(s_salt, SOILR_T);
  loadFDRS(s_soil, SOIL_T);
  loadFDRS(s_battery, VOLTAGE_T);
  loadFDRS(float(the_count), IT_T);

}

uint32_t readSalt() //Soil Electrodes: This code came from the LilyGo documentation.
{
  uint8_t samples = 120;
  uint32_t humi = 0;
  uint16_t array[120];
  for (int i = 0; i < samples; i++) {
    array[i] = analogRead(SALT_PIN);
    delay(2);
  }
  std::sort(array, array + samples);
  for (int i = 0; i < samples; i++) {
    if (i == 0 || i == samples - 1)continue;
    humi += array[i];
  }
  humi /= samples - 2;
  return humi;
}

uint16_t readSoil() //Soil Capacitance: This code came from the LilyGo documentation.
{
  uint16_t soil = analogRead(SOIL_PIN);
  return map(soil, 0, 4095, 100, 0);
}

float readBattery() //Battery Voltage: This code came from the LilyGo documentation.
{
  int vref = 1100;
  uint16_t volt = analogRead(BAT_ADC);
  float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);
  return battery_voltage;
}
void loop() {
}
