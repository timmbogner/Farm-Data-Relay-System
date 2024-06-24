//  FARM DATA RELAY SYSTEM
//
//  Irrigation Controller
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//
//
#include "fdrs_node_config.h"
#include <fdrs_node.h>

#define CONTROL_1 101  //Address for controller 1
#define CONTROL_2 102  //Address for controller 2
#define CONTROL_3 103  //Address for controller 3
#define CONTROL_4 104  //Address for controller 4

#define COIL_1 4   //Coil Pin 1
#define COIL_2 5   //Coil Pin 2
#define COIL_3 13  //Coil Pin 3
#define COIL_4 14  //Coil Pin 4

irrigController cont[] = { 
  [0] = { .address = CONTROL_1, .coilPin = COIL_1, .updatePending = false, .status = 0 },
  [1] = { .address = CONTROL_2, .coilPin = COIL_2, .updatePending = false, .status = 0 },
  [2] = { .address = CONTROL_3, .coilPin = COIL_3, .updatePending = false, .status = 0 },
  [3] = { .address = CONTROL_4, .coilPin = COIL_4, .updatePending = false, .status = 0 },
  // [4] = { .address = CONTROL_5, .coilPin = COIL_5, .updatePending = false, .status = 0 },
  // [5] = { .address = CONTROL_6, .coilPin = COIL_6, .updatePending = false, .status = 0 },
  // [6] = { .address = CONTROL_7, .coilPin = COIL_7, .updatePending = false, .status = 0 },
  // [7] = { .address = CONTROL_8, .coilPin = COIL_8, .updatePending = false, .status = 0 },
  // [8] = { .address = CONTROL_9, .coilPin = COIL_9, .updatePending = false, .status = 0 },
  // [9] = { .address = CONTROL_10, .coilPin = COIL_10, .updatePending = false, .status = 0 },
  // [10] = { .address = CONTROL_11, .coilPin = COIL_11, .updatePending = false, .status = 0 },
  // [11] = { .address = CONTROL_12, .coilPin = COIL_12, .updatePending = false, .status = 0 },
  // [12] = { .address = CONTROL_13, .coilPin = COIL_13, .updatePending = false, .status = 0 },
};

unsigned long statusCheck = 0;
bool isData = false;
bool newStatus = false;
uint numcontrollers;

void fdrs_recv_cb(DataReading theData) {
  DBG(String(theData.id));
  switch (theData.t) {
    case 0:  // Incoming command is to SET a value

      switch (theData.id) {
        case CONTROL_1:
          status_1 = (int)theData.d;
          isData = true;
          break;
        case CONTROL_2:
          status_2 = (int)theData.d;
          isData = true;
          break;
        case CONTROL_3:
          status_3 = (int)theData.d;
          isData = true;
          break;
        case CONTROL_4:
          status_4 = (int)theData.d;
          isData = true;
          break;
      }
      break;

    case 1:  // Incoming command is to GET a value
      switch (theData.id) {
        case CONTROL_1:
          if (digitalRead(COIL_1) == HIGH) {
            loadFDRS(1, STATUS_T, CONTROL_1);
          } else {
            loadFDRS(0, STATUS_T, CONTROL_1);
          }
          break;
        case CONTROL_2:
          if (digitalRead(COIL_2) == HIGH) {
            loadFDRS(1, STATUS_T, CONTROL_2);
          } else {
            loadFDRS(0, STATUS_T, CONTROL_2);
          }
          break;
        case CONTROL_3:
          if (digitalRead(COIL_3) == HIGH) {
            loadFDRS(1, STATUS_T, CONTROL_3);
          } else {
            loadFDRS(0, STATUS_T, CONTROL_3);
          }
          break;
        case CONTROL_4:
          if (digitalRead(COIL_4) == HIGH) {
            loadFDRS(1, STATUS_T, CONTROL_4);
          } else {
            loadFDRS(0, STATUS_T, CONTROL_4);
          }
          break;
      }
      newStatus = true;
      break;
  }
}

void checkCoils() {  // Sends back a status report for each coil pin.
  if (digitalRead(COIL_1) == HIGH) {
    loadFDRS(1, STATUS_T, CONTROL_1);
  } else {
    loadFDRS(0, STATUS_T, CONTROL_1);
  }
  if (digitalRead(COIL_2) == HIGH) {
    loadFDRS(1, STATUS_T, CONTROL_2);
  } else {
    loadFDRS(0, STATUS_T, CONTROL_2);
  }
  if (digitalRead(COIL_3) == HIGH) {
    loadFDRS(1, STATUS_T, CONTROL_3);
  } else {
    loadFDRS(0, STATUS_T, CONTROL_3);
  }
  if (digitalRead(COIL_4) == HIGH) {
    loadFDRS(1, STATUS_T, CONTROL_4);
  } else {
    loadFDRS(0, STATUS_T, CONTROL_4);
  }
  if (sendFDRS()) {
    DBG("Packet received by gateway");
  } else {
    DBG("Unable to communicate with gateway!");
  }
}

// Sets coil value according to data received in callback function
void updateCoils() {  
  for(int i = 0; i < numcontrollers; i++) {
    if(cont[i].updatePending == true) {
      if(cont[i].status == 0) {
        digitalWrite(cont[i].coilPin, OFF);
        DBG1("Address " + String(cont[i].address) + " coil pin " + String(cont[i].coilPin) + " off.");
      }
      else {
        digitalWrite(cont[i].coilPin, ON);
        DBG1("Address " + String(cont[i].address) + " coil pin " + String(cont[i].coilPin) + " on.");
      }
      if(cont[i].status >= 10) {
        DBG1("Address " + String(cont[i].address) + " coil pin " + String(cont[i].coilPin) + " on for " + String(cont[i].status) + " seconds.");
        cont[i].status = millis() + (cont[i].status * 1000); // this is the time when the coil will be commanded off
      }
      cont[i].updatePending = false;
    }
  }
}

void setup() {
  beginFDRS();
  pingFDRS(1000);
  if (addFDRS(1000, fdrs_recv_cb)) {
    subscribeFDRS(CONTROL_1);
    subscribeFDRS(CONTROL_2);
    subscribeFDRS(CONTROL_3);
    subscribeFDRS(CONTROL_4);
  } else {
    DBG("Not Connected");
  }
  
  // Register the callback function for received data
  addFDRS(fdrs_recv_cb);
  // Subscribe to Data Readings
  for(int i = 0; i < numcontrollers; i++) {
    subscribeFDRS(cont[i].address);
  }

}

void loop() {
  loopFDRS();
  if (isData) {
    isData = false;
    updateCoils();
    checkCoils();
  }
  if (newStatus) {
    newStatus = false;
    if (sendFDRS()) {
      DBG("Packet received by gateway");
    } else {
      DBG("Unable to communicate with gateway!");
    }
  }
  
  // periodically check for timer expiration on coils
  if(millis() - statusCheck > 500) {
    for(int i = 0; i < numcontrollers; i++) {
      if(cont[i].status >= 10 && (millis() > cont[i].status)) {
        cont[i].status = 0;
        digitalWrite(cont[i].coilPin, OFF);
        loadFDRS(OFF, STATUS_T, cont[i].address);
        DBG1("Address " + String(cont[i].address) + " coil pin " + String(cont[i].coilPin) + " turned off.");
        newStatus = true;
      }
    }
    statusCheck = millis();
  }

}