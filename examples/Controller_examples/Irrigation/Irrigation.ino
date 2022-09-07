//  FARM DATA RELAY SYSTEM
//
//  Irrigation Controller
//
//  Developed by Timm Bogner (bogner1@gmail.com) in Urbana, Illinois, USA.
//
//
#include "fdrs_sensor_config.h"
#include <fdrs_node.h>

#define CONTROL_1    133        //Address for controller 1
#define CONTROL_2    134        //Address for controller 2
#define CONTROL_3    135        //Address for controller 3
#define CONTROL_4    136        //Address for controller 4

#define COIL_1       5          //Coil Pin 1
#define COIL_2       4          //Coil Pin 2
#define COIL_3       4          //Coil Pin 3
#define COIL_4       4          //Coil Pin 4

int status_1 = 0;
int status_2 = 0;
int status_3 = 0;
int status_4 = 0;

bool newData = false;

void fdrs_recv_cb(DataReading theData) {
  switch (theData.id) {
    case CONTROL_1:
      status_1 = (int)theData.d;
      newData = true;
      break;
    case CONTROL_2:
      status_2 = (int)theData.d;
      newData = true;
      break;
    case CONTROL_3:
      status_3 = (int)theData.d;
      newData = true;
      break;
    case CONTROL_4:
      status_4 = (int)theData.d;
      newData = true;
      break;
  }
}
void updateCoils() {
  if (status_1) {
    digitalWrite(COIL_1, HIGH);
  } else {
    digitalWrite(COIL_1, LOW);
  }
  if (status_2) {
    digitalWrite(COIL_2, HIGH);
  } else {
    digitalWrite(COIL_2, LOW);
  }
  if (status_3) {
    digitalWrite(COIL_3, HIGH);
  } else {
    digitalWrite(COIL_3, LOW);
  }
  if (status_4) {
    digitalWrite(COIL_4, HIGH);
  } else {
    digitalWrite(COIL_4, LOW);
  }
}

void setup() {
  beginFDRS();
  if (addFDRS(1000, fdrs_recv_cb)) {
    subscribeFDRS(CONTROL_1);
    subscribeFDRS(CONTROL_2);
    subscribeFDRS(CONTROL_3);
    subscribeFDRS(CONTROL_4);
  } else {
    DBG("Not Connected");
  }
  pinMode(COIL_1, OUTPUT);
  digitalWrite(COIL_1, LOW);
  pinMode(COIL_2, OUTPUT);
  digitalWrite(COIL_2, LOW);
  pinMode(COIL_3, OUTPUT);
  digitalWrite(COIL_3, LOW);
  pinMode(COIL_4, OUTPUT);
  digitalWrite(COIL_4, LOW);

  DBG("FARM DATA RELAY SYSTEM :: Irrigation Module");

}

void loop()
{
  if (newData) {
    newData = false;
    updateCoils();
  }
}
