//  FARM DATA RELAY SYSTEM
//
//  Generic GPS Sensor
//
//  Developed by Sascha Juch (sascha.juch@gmail.com).
//  Reads in GPS data from serial and sends latitude, longitude and altitude to a gateway.
//

#include "fdrs_sensor_config.h"
#include <fdrs_sensor.h>

#define SERIAL1_RX 34 // TX pin of GPS sensor
#define SERIAL1_TX 12 // RX pin of GPS sensor
#define MAX_NMEA_LENGTH 82 //maximum allowed length of a NMEA 0183 sentences.

char currentNMEALine[MAX_NMEA_LENGTH];

void setup() {
  // ToDo: This works well on a board with a second hardware serial port like the ESP32. But what if there is no hardware serial on the device?
  // Unfortunately I do not have a GPS (standalone) sensor atm with which I could test. Help and advice appreciated.  
  Serial1.begin(9600, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX);
  
  beginFDRS();
}

void loop() {
  // read in line by line of the NMEA input and get rid of trailing whitespaces
  Serial1.readBytesUntil('\n', currentNMEALine, MAX_NMEA_LENGTH);
  trimwhitespace(currentNMEALine);
  
  // we are only interested in GPGGA (U-Blox M6N) or GNGGA (U-Blox M8N)lines.  
  if (startsWith(currentNMEALine, "$GNGGA") || startsWith(currentNMEALine, "$GPGGA")) {
    
    DBG(currentNMEALine);
  
    // just in case someone needs UTC, quality or #satelites, just uncomment and do what you have to do with them. :)
    //char * gpsUTC                   = getNthValueOf(currentNMEALine, ',', 1);
    char * gpsLatitude              = getNthValueOf(currentNMEALine, ',', 2);
    char * gpsLatitudeOrientation   = getNthValueOf(currentNMEALine, ',', 3);
    char * gpsLongitude             = getNthValueOf(currentNMEALine, ',', 4);
    char * gpsLongitudeOrientation  = getNthValueOf(currentNMEALine, ',', 5);
    //char * gpsQuality               = getNthValueOf(currentNMEALine, ',', 6);
    char * gpsAltitude              = getNthValueOf(currentNMEALine, ',', 7);
    //char * gpsNoOfSatelites         = getNthValueOf(currentNMEALine, ',', 9);
  
    // convert latitude and altitude to decimal degree values (as used in most maps programs)
    // negative values mean "S" or "W", positive values mean "N" and "E"
    float latitude = convertGpsCoordinates(atof(gpsLatitude), gpsLatitudeOrientation);
    float longitude = convertGpsCoordinates(atof(gpsLongitude), gpsLongitudeOrientation);
    float altitude = atof(gpsAltitude);
      
/*
    loadFDRS(latitude, HUMIDITY_T);
    loadFDRS(longitude, TEMP_T);
    loadFDRS(altitude, TEMP2_T);
*/

    // extended sensor types - not officially atm!    
    loadFDRS(latitude, LATITUDE_T);
    loadFDRS(longitude, LONGITUDE_T);
    loadFDRS(altitude, ALTITUDE_T);
    
    
    sendFDRS();
    sleepFDRS(10);  //Sleep time in seconds
  }

}

// cudos for the trimming function go to: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
// Thanks! That was a time saver. :)
// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// check, if a given char* fullString starts with a given char* startString.
// If that's the case, return true, false otherwise
bool startsWith(const char *fullString, const char *startString)
{
   if (strncmp(fullString, startString, strlen(startString)) == 0) return 1;
   return 0;
}

// Cudos for the substr function go to: https://www.techiedelight.com/implement-substr-function-c/ 
// Thanks! That helped a lot :)
// Following function extracts characters present in `src`
// between `m` and `n` (excluding `n`)
char* substr(const char *src, int m, int n)
{
    // get the length of the destination string
    int len = n - m;
 
    // allocate (len + 1) chars for destination (+1 for extra null character)
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
 
    // extracts characters between m'th and n'th index from source string
    // and copy them into the destination string
    for (int i = m; i < n && (*(src + i) != '\0'); i++)
    {
        *dest = *(src + i);
        dest++;
    }
 
    // null-terminate the destination string
    *dest = '\0';
 
    // return the destination string
    return dest - len;
}

// returns the value of the n-th occurance within a delimiter-separated string
char * getNthValueOf (char *inputString, const char delimiter, uint8_t index) {
  uint8_t i = 0;
  uint8_t currentIndex = 0;
  uint8_t startOfValue = 0;
  uint8_t endOfValue = 0;

  while (i < strlen(inputString) && inputString[i] && currentIndex < index) {
    if (inputString[i] == delimiter) {
      currentIndex++;
    }
    i++;
  }
  startOfValue = i;

  while (i < strlen(inputString) && inputString[i] && currentIndex <= index) {
    if (inputString[i] == delimiter) {
      currentIndex++;
    }
    i++;
  }
  endOfValue = i;

  char* valueAtIndex = substr(inputString, startOfValue, endOfValue-1);
  
  return valueAtIndex;
}

// convert NMEA0183 degrees minutes coordinates to decimal degrees
float convertGpsCoordinates(float degreesMinutes, char* orientation) {
  double gpsMinutes = fmod((double)degreesMinutes, 100.0);
  uint8_t gpsDegrees = degreesMinutes / 100;
  double decimalDegrees = gpsDegrees + ( gpsMinutes / 60 );
  if (orientation == "W" || orientation == "S") {
    decimalDegrees = 0 - decimalDegrees;
  }
  return decimalDegrees;
}
