//  FARM DATA RELAY SYSTEM
//
//  Example sketch to provide several examples to obtain and use time
//  from a gateway
//   This sketch retrieves the time from a nearby FDRS gateway using ESP-NOW.
//    After retrieving the time, at 60 second intervals, it prints out each field of the time
//    in two different methods.
//  
//  Developed by Timm Bogner (timmbogner@gmail.com) in Urbana, Illinois, USA
//  FDRS Timekeeping was developed and contributed by Jeff Lehman (aviateur17)
//  
//

#include "fdrs_node_config.h"
#include <fdrs_node.h>

unsigned long printJob = 0;
struct tm timeData;
char strftime_buf[80];

static char dayNames[][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static char monthNames[][12] = { "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December" };
bool isPM = false;

void setup() {
  DBG("FDRS ControllerTime example.");

  beginFDRS();

  // Request that the gateway send us the current time
  DBG("Requesting time from gateway");
  reqTimeFDRS();
  delay(1000);
}

void loop() {
  loopFDRS();

    // If time is valid, prints time out every 60 seconds (60,000 ms)
  if(millis() - printJob > (1000 * 60) || printJob == 0) 
  {
    // Check to see that we have a valid time
    if(validTime()) 
    {
      // time is valid
      
      DBG("");
      DBG("Configured Standard Time offset from UTC: " + String(STD_OFFSET) + " hours.");
      DBG("Configured Daylight Savings Time offset from UTC: " + String(DST_OFFSET) + " hours.");

      // Option 1 - use strftime function  
      // Local time
      // time is stored in UTC so add offset to convert to local time
      time_t localt = time(NULL) + (isDST?dstOffset:stdOffset);
      // convert from time_t format to struct tm format
      localtime_r(&localt, &timeData);
      // Store the date/time in a character array
      // https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeData);
      // Print out the character array to display the local date/time
      DBG("---- strftime() function output ----");
      DBG("Local date/time is: " + String(strftime_buf) + (isDST?" DST":" STD"));
      
      strftime(strftime_buf, sizeof(strftime_buf), "%Y", &timeData);
      DBG("Year: " + String(strftime_buf));               // %Y - year in 4 digits
      strftime(strftime_buf, sizeof(strftime_buf), "%m", &timeData);
      DBG("Month: " + String(strftime_buf));              // %m - Month as a decimal number
      strftime(strftime_buf, sizeof(strftime_buf), "%B", &timeData);
      DBG("Month Name: " + String(strftime_buf));         // %B - Month Full Name
      strftime(strftime_buf, sizeof(strftime_buf), "%d", &timeData);
      DBG("Day of Month: " + String(strftime_buf));       // %d - day of the month (1 - 31)
      strftime(strftime_buf, sizeof(strftime_buf), "%w", &timeData);
      DBG("Day of Week: " + String(strftime_buf));        // %w - Days since Sunday = 0
      strftime(strftime_buf, sizeof(strftime_buf), "%A", &timeData);
      DBG("Day Name: " + String(strftime_buf));           // %A - Full weekday name
      strftime(strftime_buf, sizeof(strftime_buf), "%j", &timeData);
      DBG("Day of Year: " + String(strftime_buf));        // %j - days since January 1st = 0
      strftime(strftime_buf, sizeof(strftime_buf), "%H", &timeData);
      DBG("Hour (24 hour format): " + String(strftime_buf)); // %H - Hours since mindnight in 24 hour format
      strftime(strftime_buf, sizeof(strftime_buf), "%I", &timeData);
      DBG("Hour (12 hour format): " + String(strftime_buf)); // %I - Hours since mindnight in 12 hour format
      strftime(strftime_buf, sizeof(strftime_buf), "%M", &timeData);
      DBG("Minute: " + String(strftime_buf));               // %M - Minutes
      strftime(strftime_buf, sizeof(strftime_buf), "%S", &timeData);
      DBG("Second: " + String(strftime_buf));               // %S - Seconds
      strftime(strftime_buf, sizeof(strftime_buf), "%p", &timeData);
      DBG("AM/PM: " + String(strftime_buf));                // %p - AM/PM
      DBG("Daylight Savings: " + String((isDST ? " DST (yes)" : " STD (no)")));
      strftime(strftime_buf, sizeof(strftime_buf), "%A, %m/%d/%Y %H:%M:%S %p", &timeData);
      DBG("US format: " + String(strftime_buf) + String((isDST ? " DST" : " STD")));      // Day, MM/DD/YYYY HH:MM:SS AM/PM DST/STD
      strftime(strftime_buf, sizeof(strftime_buf), "%A, %d/%m/%Y %I:%M:%S", &timeData);
      DBG("European format: " + String(strftime_buf) + String((isDST ? " DST" : " STD")));      // Day, DD/MM/YYYY HH:MM:SS DST/STD
      


      // Option 2 - read directly from the tm struct
      // Local time
      // time is stored in UTC so add offset to convert to local time
      localt = time(NULL) + (isDST?dstOffset:stdOffset);
      // convert from time_t format to struct tm format
      localtime_r(&localt, &timeData);
      // Struct tm provides the date/time elements split out
      // https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
      DBG("---- struct tm output ----");
      DBG("Year: " + String(timeData.tm_year + 1900)); // tm_year returns number of years since 1900 so we add 1900
      DBG("Month: " + String(timeData.tm_mon + 1)); // Jan = 0 so add 1 to (1 - 12)
      DBG("Month Name: " + String(monthNames[timeData.tm_mon]));
      DBG("Day of Month: " + String(timeData.tm_mday)); // day of the month (1 - 31)
      DBG("Day of Week: " + String(timeData.tm_wday)); // Days since Sunday = 0
      DBG("Day Name: " + String(dayNames[timeData.tm_wday]));
      DBG("Day of Year: " + String(timeData.tm_yday + 1)); // days since January 1st = 0
      DBG("Hour (24 hour format): " + String(timeData.tm_hour)); // Hours since mindnight in 24 hour format
      if(timeData.tm_hour >= 12 && timeData.tm_hour <= 23) 
      {
        isPM = true;
      }
      else 
      {
        isPM = false;
      }
      if(timeData.tm_hour >= 13)
      {
        timeData.tm_hour = timeData.tm_hour - 12;
      }
      DBG("Hour (12 hour format): " + String(timeData.tm_hour) + " " + String((isPM ? "PM" : "AM"))); // Hours since mindnight in 12 hour format
      DBG("Minute: " + String(timeData.tm_min));
      DBG("Second: " + String(timeData.tm_sec));
      DBG("Daylight Savings: " + String((isDST ? " DST (yes)" : " STD (no)")));
      DBG("Local Time: " + String(dayNames[timeData.tm_wday]) + ", " + String(timeData.tm_mon + 1) + "/" + String(timeData.tm_mday) + "/"
           + String(timeData.tm_year + 1900) + " " + String(timeData.tm_hour) + ":" + String(timeData.tm_min) + ":" + String(timeData.tm_sec)
           + String((isPM ? " PM" : " AM")) + String((isDST ? " DST" : " STD")));
      DBG("");
      DBG("");

      printJob = millis();
    }
    else // Time is not valid. Request time from the gateway
    {
      DBG("Time not valid.");
      DBG("Check gateway is running and has valid time.");
      DBG("");
      reqTimeFDRS();

      // Try again in 20 seconds
      printJob = printJob + 20000;
    }
  }
}