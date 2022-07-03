scriptdir=$(dirname $BASH_SOURCE)
cd $scriptdir

cp ../FDRS_Sensor/fdrs_sensor.h ../Examples/1_LoRa_Sensor/
cp ../FDRS_Sensor/fdrs_sensor.h ../Examples/2_ESPNOW_Sensor/
cp ../FDRS_Sensor/fdrs_sensor.h ../Examples/ESPNOW_Stress_Test/

cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/AHT20_fdrs/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/BME280_fdrs/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/BMP280_fdrs/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/DHT22_fdrs/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/DS18B20_fdrs/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/LilyGo_HiGrow_32/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/MESB_fdrs/
cp ../FDRS_Sensor/fdrs_sensor.h ../Sensors/TippingBucket/

cp ../FDRS_Gateway/fdrs_functions.h ../Examples/3_ESPNOW_Gateway/
cp ../FDRS_Gateway/fdrs_functions.h ../Examples/4_UART_Gateway/
cp ../FDRS_Gateway/fdrs_functions.h ../Examples/5_MQTT_Gateway/
