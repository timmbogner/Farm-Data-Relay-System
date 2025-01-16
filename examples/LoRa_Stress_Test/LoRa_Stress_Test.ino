//  FARM DATA RELAY SYSTEM
//
//  LoRa Stress Tester or "Spammer"
//  
//

#include "fdrs_node_config.h"
#include <fdrs_node.h>

#define PING_TIME_DELAY 300
#define REQ_TIME_DELAY 300
#define LOAD_TIME_DELAY 1000
#define CYCLES 15
unsigned long lastruntime = 0;
int test = 0;
unsigned long lastCycle = 0;

void setup() {
  beginFDRS();
}
void loop() {
  loopFDRS();

  switch (test) {
  case 0:
    DBG("Sending pings in quick succession.");
    delay(4000);
    for(int i=1; i < CYCLES + 1; i) {
      if(TDIFF(lastruntime,PING_TIME_DELAY)) {
        lastruntime = millis();
        Serial.println();
        DBG("--- Test #" + String(i) + " ---");
        Serial.println();
        pingFDRS(1000);
        i++;
      }
      loopFDRS();
    }
    while(millis()-lastruntime < 1000) {
      loopFDRS();
    }
    test++;
    Serial.println();
    Serial.println();
    break;
  
  case 1:
    DBG("Sending time requests in quick succession.");
    delay(4000);
    for(int i=1; i < CYCLES + 1; i) {
      if(TDIFF(lastruntime,REQ_TIME_DELAY)) {
        lastruntime = millis();
        Serial.println();
        DBG("--- Test #" + String(i) + " ---");
        Serial.println();
        reqTimeFDRS();
        i++;
      }
      loopFDRS();
    }
    while(millis()-lastruntime < 1000) {
      loopFDRS();
    }
    test++;
    Serial.println();
    Serial.println();
    break;
  
  case 2:
    DBG("No ACK - data cycles in quick succession.");
    ack = false;
    delay(4000);
    for(int i=1; i < CYCLES + 1; i) {
      if(TDIFF(lastruntime,LOAD_TIME_DELAY)) {
        lastruntime = millis();
        Serial.println();
        DBG("--- Test #" + String(i) + " ---");
        Serial.println();
        for(int j=0; j < i; j++) {
          loadFDRS(READING_ID + j, STATUS_T);
        }
        if(sendFDRS()){
          DBG("Big Success!");
        } else {
          DBG("Nope, not so much.");
        }
        i++;
      }
      loopFDRS();
    }
    while(millis()-lastruntime < 1000) {
      loopFDRS();
    }
    test++;
    Serial.println();
    Serial.println();
    break;
  
  case 3:
    DBG("ACK - data cycles in quick succession.");
    ack = true;
    delay(4000);
    for(int i=1; i < CYCLES + 1; i) {
      if(TDIFF(lastruntime,LOAD_TIME_DELAY)) {
        lastruntime = millis();
        Serial.println();
        DBG("--- Test #" + String(i) + " ---");
        Serial.println();
        for(int j=0; j < i; j++) {
          loadFDRS(READING_ID + j, STATUS_T);
        }
        if(sendFDRS()){
          DBG("Big Success!");
        } else {
          DBG("Nope, not so much.");
        }
        i++;
      }
      loopFDRS();
    }
    while(millis()-lastruntime < 1000) {
      loopFDRS();
    }
    test++;
    Serial.println();
    Serial.println();
    break;

case 4:
    DBG("Combination of commands in quick succession.");
    delay(4000);
    for(int i=1; i < CYCLES + 1; i) {
      if(TDIFF(lastruntime,5000)) {
        lastruntime = millis();
        Serial.println();
        DBG("--- Test #" + String(i) + " ---");
        Serial.println();
        pingFDRS(1000);
        reqTimeFDRS();
        subscribeFDRS(READING_ID);
        i++;
      }
      loopFDRS();
    }
    while(millis()-lastruntime < 2000) {
      loopFDRS();
    }
    test++;
    Serial.println();
    Serial.println();
    break;

  default:
    DBG("DONE!  Starting over again soon.");
    Serial.println();
    Serial.println();
    delay(4000);
    test = 0;
    break;
  }
}
