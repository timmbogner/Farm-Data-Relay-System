//  FARM DATA RELAY SYSTEM
//
//  LILYGO HIGROW SENSOR MODULE
//
//  Developed by Timm Bogner (bogner1@gmail.com) for Sola Gratia Farm in Urbana, Illinois, USA.
//  Each sensor is assigned a two-byte identifier along with a one-byte sensor type

#define TERM_MAC    0x00 //Terminal MAC
#define SLEEPYTIME  60   //Time to sleep in seconds

#define TEMP_ID     18    //Unique ID (0 - 1023) for each data reading
#define HUM_ID      19
#define LUX_ID      20
#define PRES_ID     21
#define SALT_ID     22
#define SOIL_ID     23
#define BAT_ID      24
#define COUNT_ID    25

#define I2C_SDA             25
#define I2C_SCL             26
#define DHT12_PIN           16
#define BAT_ADC             33
#define SALT_PIN            34
#define SOIL_PIN            32
#define BOOT_PIN            0
#define POWER_CTRL          4
#define USER_BUTTON         35
#define DS18B20_PIN         21

#include <WiFi.h>
#include <esp_now.h>
#include <BH1750.h>
#include <Adafruit_BME280.h>
#include "DataReading.h"

BH1750 lightMeter(0x23); //0x23
Adafruit_BME280 bmp;     //0x77
RTC_DATA_ATTR int the_count = 0;
uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, TERM_MAC};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //  Serial.print("Last Packet Send Status:");
  //  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  esp_deep_sleep_start();
}

void setup() {
  //Serial.begin(115200);
  // Sensor power control pin, set high to enable sensors.
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
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

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  loadData();
  digitalWrite(POWER_CTRL, 0);
  esp_sleep_enable_timer_wakeup(SLEEPYTIME * 1000000);

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
  Serial.println("Temp:" + String(bme_temp));
  Serial.println("Humidity:" + String(bme_humidity));
  Serial.println("Light:" + String(lux));
  Serial.println("Pressure:" + String(bme_pressure));
  Serial.println("Salt:" + String(s_salt));
  Serial.println("Soil:" + String(s_soil));
  Serial.println("Voltage:" + String(s_battery));
  Serial.println("Count:" + String(the_count));

  DataReading Temp;
  Temp.d = bme_temp;
  Temp.id = TEMP_ID;
  Temp.t = 1;

  DataReading Hum;
  Hum.d = bme_humidity;
  Hum.id = HUM_ID;
  Hum.t = 2;

  DataReading Lux;
  Lux.d = lux;
  Lux.id = LUX_ID;
  Lux.t = 4;

  DataReading Pres;
  Pres.d = bme_pressure;
  Pres.id = PRES_ID;
  Pres.t = 3;

  DataReading Salt;
  Salt.d = s_salt;
  Salt.id = SALT_ID;
  Salt.t = 6;

  DataReading Soil;
  Soil.d = s_soil;
  Soil.id = SOIL_ID;
  Soil.t = 5;

  DataReading Bat;
  Bat.d = s_battery;
  Bat.id = BAT_ID;
  Bat.t = 50;

  DataReading Count;
  Count.d = float(the_count);
  Count.id = COUNT_ID;
  Count.t = 51;

  DataReading thePacket[8];
  thePacket[0] = Temp;
  thePacket[1] = Hum;
  thePacket[2] = Lux;
  thePacket[3] = Pres;
  thePacket[4] = Salt;
  thePacket[5] = Soil;
  thePacket[6] = Bat;
  thePacket[7] = Count;
  //Serial.println(sizeof(thePacket), DEC);

  esp_now_send(broadcastAddress, (uint8_t *) &thePacket, sizeof(thePacket));
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
