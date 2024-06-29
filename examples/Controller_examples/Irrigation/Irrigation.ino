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
  bool updatePending = false;
  unsigned long status = 0;
} irrigController;

/*

Format of loadFDRS....
loadFDRS(data, type, address);

Get Coil status of address 102 (data value is ignored)
loadFDRS(1, 1, 102);  

Turn off (set) Coil at address 102 for an indefinite amount of time
loadFDRS(0, 0, 102);  

Turn on (set) Coil at address 102 for an indefinite amount of time
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

#define COIL_1 4   //Coil Pin 1
#define COIL_2 5   //Coil Pin 2
#define COIL_3 13  //Coil Pin 3
#define COIL_4 14  //Coil Pin 4

// These are set up for relay module which are active-LOW. 
// Swap 'HIGH'and 'LOW' to use the inverse.
#define ON LOW
#define OFF HIGH

irrigController coil[] = { 
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
        if(coil[i].address == (uint) theData.id) {
          coil[i].status = (unsigned long) theData.d;
          coil[i].updatePending = true;
          isData = true;
          DBG1("Received SET cmd. Address: " + String(theData.id) + " value: " + String(theData.d));
          break;
        }
      }
      break;

    case CMD_GET:  // Incoming command is to GET a value
      for(int i = 0; i < numcontrollers; i++) {
        if(coil[i].address == theData.id) {
          if (digitalRead(coil[i].coilPin) == HIGH) {
            loadFDRS(1, STATUS_T, coil[i].address);
          } else {
            loadFDRS(0, STATUS_T, coil[i].address);
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
    if (digitalRead(coil[i].coilPin) == HIGH) {
      loadFDRS(1, STATUS_T, coil[i].address);
    } else {
      loadFDRS(0, STATUS_T, coil[i].address);
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
    if(coil[i].updatePending == true) {
      if(coil[i].status == 0) {
        digitalWrite(coil[i].coilPin, OFF);
        delay(10);
        bool outVal = digitalRead(coil[i].coilPin);
        DBG1("Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " commanded off. Status: " + (outVal?"HIGH":"LOW"));
        if(outVal != OFF) {
          DBG("Alert! Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " commanded off but remains on!");
        }
      }
      else {
        digitalWrite(coil[i].coilPin, ON);
        delay(10);
        bool outVal = digitalRead(coil[i].coilPin);
        DBG1("Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " commanded off. Status: " + (outVal?"HIGH":"LOW"));
        if(outVal != ON) {
          DBG("Alert! Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " commanded on but remains off!");
        }
      }
      if(coil[i].status >= 10) {
        DBG1("Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " on for " + String(coil[i].status) + " seconds.");
        coil[i].status = millis() + (coil[i].status * 1000); // this is the time when the coil will be commanded off
      }
      coil[i].updatePending = false;
    }
  }
}

void setup() {
  beginFDRS();
  DBG("FARM DATA RELAY SYSTEM :: Irrigation Module");
  pingFDRS(1000);

  numcontrollers = (uint) sizeof(coil)/sizeof(irrigController);
  // set up the physical outputs
  for(int i = 0; i < numcontrollers; i++) {
    pinMode(coil[i].coilPin, OUTPUT);
    digitalWrite(coil[i].coilPin, OFF);
  }

  // Register the callback function for received data
  if(addFDRS(fdrs_recv_cb)) {
    // Subscribe to Data Readings
    for(int i = 0; i < numcontrollers; i++) {
      subscribeFDRS(coil[i].address);
    }
  } else {
    DBG("Not Connected");
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
      if(coil[i].status >= 10 && (millis() > coil[i].status)) {
        coil[i].status = 0;
        digitalWrite(coil[i].coilPin, OFF);
        loadFDRS(OFF, STATUS_T, coil[i].address);
        delay(10);
        bool outVal = digitalRead(coil[i].coilPin);
        DBG1("Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " commanded off. Status: " + (outVal?"HIGH":"LOW"));
        if(outVal != OFF) {
          DBG("Alert! Address " + String(coil[i].address) + " coil pin " + String(coil[i].coilPin) + " commanded off but remains on!");
        }
        newStatus = true;
      }
    }
    statusCheck = millis();
  }
}