#define READING_ID 31
#define GTWY_MAC 0x01
#define USE_ESPNOW
#define COIL_PIN 5

#include <fdrs_node.h>

bool status = 0;

void fdrs_recv_cb(DataReading theData) {
  status = (bool)theData.d;
}
void setup() {
  beginFDRS();
  if (addFDRS(1000, fdrs_recv_cb))
    subscribeFDRS(READING_ID);
  pinMode(COIL_PIN, OUTPUT);
}
void loop() {
  if (status) digitalWrite(COIL_PIN, LOW);
  else digitalWrite(COIL_PIN, HIGH);
}