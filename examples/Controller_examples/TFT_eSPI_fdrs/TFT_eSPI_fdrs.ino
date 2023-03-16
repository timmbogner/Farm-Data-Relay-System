//  FARM DATA RELAY SYSTEM
//
//  TFT_eSPI Example
//
//  Listens for datareading IDs #101-104 and displays their data in the four corners of a TFT screen.
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA.
//
//
#include "fdrs_node_config.h"
#include <fdrs_node.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

#define CONTROL_A    101        //ID for datapoint A
#define CONTROL_B    102        //ID for datapoint B
#define CONTROL_C    103        //ID for datapoint C
#define CONTROL_D    104        //ID for datapoint D

float value_A = 0;
float value_B = 0;
float value_C = 0;
float value_D = 0;

bool newData = false;

void fdrs_recv_cb(DataReading theData) {
  switch (theData.id) {
    case CONTROL_A:
      value_A = theData.d;
      newData = true;
      break;
    case CONTROL_B:
      value_B = theData.d;
      newData = true;
      break;
    case CONTROL_C:
      value_C = theData.d;
      newData = true;
      break;
    case CONTROL_D:
      value_D = theData.d;
      newData = true;
      break;
  }
}
void updateScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawFloat(value_A, 3, 0, 0, 2);
  tft.setTextDatum(TR_DATUM);
  tft.drawFloat(value_B, 3, tft.width(), 0, 2);
  tft.setTextDatum(BL_DATUM);
  tft.drawFloat(value_C, 3, 0, tft.height(), 2);
  tft.setTextDatum(BR_DATUM);
  tft.drawFloat(value_D, 3, tft.width(), tft.height(), 2);

}

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  updateScreen();
  beginFDRS();
    DBG("W:" +String(tft.width()));
  DBG("FARM DATA RELAY SYSTEM :: TFT_eSPI Example -- thanks Bodmer!");
  if (addFDRS(1000, fdrs_recv_cb)) {
    subscribeFDRS(CONTROL_A);
    subscribeFDRS(CONTROL_B);
    subscribeFDRS(CONTROL_C);
    subscribeFDRS(CONTROL_D);
  } else {
    DBG("Not Connected");
  }

}

void loop()
{
  loopFDRS();
  if (newData) {
    newData = false;
    updateScreen();
  }
}
