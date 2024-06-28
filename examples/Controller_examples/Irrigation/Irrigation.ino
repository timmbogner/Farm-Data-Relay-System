//  FARM DATA RELAY SYSTEM
//
//  Irrigation Controller
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//  Updated by Jeff Lehman
//

#include "fdrs_node_config.h"
#include <fdrs_node.h>

#define CMD_GET 1
#define CMD_SET 0

// These are set up for relay module which are active-LOW. 
// Swap 'HIGH'and 'LOW' to use the inverse.
#define ON LOW
#define OFF HIGH

typedef struct irrigController
{
  uint address;
  int coilPin;
  bool updatePending = false;
  unsigned long status = 0;
} irrigController;

/*
Examples:

Turn on coil address 102 for 30 seconds:
[{"id":102,"type":0,"data":15}]

Turn on coil address 102, and leave it on permanently:
[{"id":102,"type":0,"data":1}]

Turn off coil address 102:
[{"id":102,"type":0,"data":0}]

Return the status of coil address 102 (data portion is ignored):
[{"id":102,"type":1,"data":0}]

Using the timer function:
Data value represents seconds to remain on. Must be 10 or greater.

After setting any value, the controller will send back the status of all coils.
*/

#define CONTROL_1 101  //Address for controller 1
#define CONTROL_2 102  //Address for controller 2
#define CONTROL_3 103  //Address for controller 3
#define CONTROL_4 104  //Address for controller 4

#define COIL_1 GPIO_NUM_16   //Coil Pin 1
#define COIL_2 GPIO_NUM_17   //Coil Pin 2
#define COIL_3 GPIO_NUM_18  //Coil Pin 3
#define COIL_4 GPIO_NUM_19  //Coil Pin 4

irrigController coils[] = { 
  [0] = { .address = CONTROL_1, .coilPin = COIL_1 },
  [1] = { .address = CONTROL_2, .coilPin = COIL_2 },
  [2] = { .address = CONTROL_3, .coilPin = COIL_3 },
  [3] = { .address = CONTROL_4, .coilPin = COIL_4 },
  // [4] = { .address = CONTROL_5, .coilPin = COIL_5 },
  // [5] = { .address = CONTROL_6, .coilPin = COIL_6 },
  // [6] = { .address = CONTROL_7, .coilPin = COIL_7 },
  // [7] = { .address = CONTROL_8, .coilPin = COIL_8 },
  // [8] = { .address = CONTROL_9, .coilPin = COIL_9 },
  // [9] = { .address = CONTROL_10, .coilPin = COIL_10 },
  // [10] = { .address = CONTROL_11, .coilPin = COIL_11 },
  // [11] = { .address = CONTROL_12, .coilPin = COIL_12 },
  // [12] = { .address = CONTROL_13, .coilPin = COIL_13 },
};

unsigned long statusCheck = 0;
bool isData = false;
bool newStatus = false;
uint numcontrollers;

// Callback function in the controller that receives data to get or set coils
void fdrs_recv_cb(DataReading theData) {

  switch (theData.t) {
    case CMD_SET:  // Incoming command is to SET a value
      for(int i = 0; i < numcontrollers; i++) {
        if(coils[i].address == (uint) theData.id) {
          coils[i].status = (unsigned long) theData.d;
          coils[i].updatePending = true;
          isData = true;
          DBG1("Received SET cmd. Address: " + String(theData.id) + " value: " + String(theData.d));
          break;
        }
      }
      break;

    case CMD_GET:  // Incoming command is to GET a value
      for(int i = 0; i < numcontrollers; i++) {
        if(coils[i].address == theData.id) {
          if (digitalRead(coils[i].coilPin) == HIGH) {
            loadFDRS(1, STATUS_T, coils[i].address);
          } else {
            loadFDRS(0, STATUS_T, coils[i].address);
          }
          DBG1("Received GET cmd for address: " + String(theData.id));
          newStatus = true;
        }
      }
      break;
    
    default:
      DBG1("Unknown command: " + String(theData.t) + " address: " + String(theData.id) + " value: " + String(theData.d));
      break;

  }
}

void checkCoils() {  // Sends back a status report for each coil pin.
  for(int i = 0; i < numcontrollers; i++) {
    if (digitalRead(coils[i].coilPin) == HIGH) {
      loadFDRS(1, STATUS_T, coils[i].address);
    } else {
      loadFDRS(0, STATUS_T, coils[i].address);
    }
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
    if(coils[i].updatePending == true) {
      if(coils[i].status == 0) {
        digitalWrite(coils[i].coilPin, OFF);
        DBG1("Address " + String(coils[i].address) + " coil pin " + String(coils[i].coilPin) + " off.");
      }
      else {
        digitalWrite(coils[i].coilPin, ON);
        DBG1("Address " + String(coils[i].address) + " coil pin " + String(coils[i].coilPin) + " on.");
      }
      if(coils[i].status >= 10) {
        DBG1("Address " + String(coils[i].address) + " coil pin " + String(coils[i].coilPin) + " on for " + String(coils[i].status) + " seconds.");
        coils[i].status = millis() + (coils[i].status * 1000); // this is the time when the coil will be commanded off
      }
      coils[i].updatePending = false;
    }
  }
}

void setup() {
  beginFDRS();
  DBG("FARM DATA RELAY SYSTEM :: Irrigation Module");
  pingFDRS(1000);

  numcontrollers = (uint) sizeof(coils)/sizeof(irrigController);
  // set up the physical outputs
  for(int i = 0; i < numcontrollers; i++) {
    pinMode(coils[i].coilPin, OUTPUT);
    digitalWrite(coils[i].coilPin, OFF);
  }
  
  // Register the callback function for received data
  addFDRS(fdrs_recv_cb);
  // Subscribe to Data Readings
  for(int i = 0; i < numcontrollers; i++) {
    subscribeFDRS(coils[i].address);
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
      if(coils[i].status >= 10 && (millis() > coils[i].status)) {
        coils[i].status = 0;
        digitalWrite(coils[i].coilPin, OFF);
        loadFDRS(OFF, STATUS_T, coils[i].address);
        DBG1("Address " + String(coils[i].address) + " coil pin " + String(coils[i].coilPin) + " turned off.");
        newStatus = true;
      }
    }
    statusCheck = millis();
  }
}