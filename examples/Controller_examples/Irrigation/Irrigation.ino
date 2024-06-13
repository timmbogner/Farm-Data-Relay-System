//  FARM DATA RELAY SYSTEM
//
//  Irrigation Controller
//
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//
//

#include "fdrs_node_config.h"
#include <fdrs_node.h>

#define CMD_GET 1
#define CMD_SET 0

typedef struct irrigController
{
  uint address;
  int coilPin;
  bool updatePending;
  unsigned long status;
} irrigController;

/*

Format of loadFDRS....
loadFDRS(data, type, address);

Get Coil status of address 102 (data value is ignored)
loadFDRS(1, 1, 102);  

Turn off (set) Coil at address 102 for an indefinite amount of time
loadFDRS(0, 0, 102);  

Turn on Coil (set) at address 102 for an indefinite amount of time
loadFDRS(1, 0, 102);  

Turn on (set) Coil at address 102 for 300 seconds
loadFDRS(300, 0, 102);  

When turning on coil for certain amount of time the data value
must be 10 or greater and is in units of seconds.

*/

#define CONTROL_1 101  //Address for controller 1
#define CONTROL_2 102  //Address for controller 2
#define CONTROL_3 103  //Address for controller 3
#define CONTROL_4 104  //Address for controller 4

#define COIL_1 GPIO_NUM_16   //Coil Pin 1
#define COIL_2 GPIO_NUM_17   //Coil Pin 2
#define COIL_3 GPIO_NUM_18  //Coil Pin 3
#define COIL_4 GPIO_NUM_19  //Coil Pin 4

// These are set up for relay module which are active-LOW. 
// Swap 'HIGH'and 'LOW' to use the inverse.
#define ON LOW
#define OFF HIGH

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

// Callback function in the controller that receives data to get or set coils
void fdrs_recv_cb(DataReading theData) {

  switch (theData.t) {
    case CMD_SET:  // Incoming command is to SET a value
      for(int i = 0; i < numcontrollers; i++) {
        if(cont[i].address == (uint) theData.id) {
          cont[i].status = (unsigned long) theData.d;
          cont[i].updatePending = true;
          isData = true;
          DBG1("Received SET cmd. Address: " + String(theData.id) + " value: " + String(theData.d));
          break;
        }
      }
      break;

    case CMD_GET:  // Incoming command is to GET a value
      for(int i = 0; i < numcontrollers; i++) {
        if(cont[i].address == theData.id) {
          if (digitalRead(cont[i].coilPin) == HIGH) {
            loadFDRS(1, STATUS_T, cont[i].address);
          } else {
            loadFDRS(0, STATUS_T, cont[i].address);
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
    if (digitalRead(cont[i].coilPin == HIGH)) {
      loadFDRS(1, STATUS_T, cont[i].address);
    } else {
      loadFDRS(0, STATUS_T, cont[i].address);
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
  DBG("FARM DATA RELAY SYSTEM :: Irrigation Module");
  pingFDRS(1000);

  numcontrollers = (uint) sizeof(cont)/sizeof(irrigController);
  // set up the physical outputs
  for(int i = 0; i < numcontrollers; i++) {
    pinMode(cont[i].coilPin, OUTPUT);
    digitalWrite(cont[i].coilPin, OFF);
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