

#ifdef USE_SD_LOG
#include <SPI.h>
#include <SD.h>
#endif
#ifdef USE_FS_LOG
#include <LittleFS.h>
#endif
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
#include <time.h>
#endif
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
char logBuffer[512];
uint16_t logBufferPos = 0; // datatype depends on size of sdBuffer
uint32_t timeLOGBUF = 0;
time_t last_mqtt_success = 0;
time_t last_log_write = 0;
void handleLogger()
{
  if ((millis() - timeLOGBUF) >= LOGBUF_DELAY)
  {
    timeLOGBUF = millis();
    if (logBufferPos > 0)
      releaseLogBuffer();
  }
}

#endif

#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
void releaseLogBuffer()
{
#ifdef USE_SD_LOG
  DBG("Releasing Log buffer to SD");
  File logfile = SD.open(LOG_FILENAME, FILE_WRITE);
  if ((logfile.size() / 1024.0) < SD_MAX_FILESIZE)
  {
    logfile.print(logBuffer);
  }
  logfile.close();
#endif
#ifdef USE_FS_LOG
  DBG("Releasing Log buffer to internal flash.");
  File logfile = LittleFS.open(FS_FILENAME, "a");
  if ((logfile.size() / 1024.0) < FS_MAX_FILESIZE)
  {
    logfile.print(logBuffer);
  }
  logfile.close();
#endif
  memset(&(logBuffer[0]), 0, sizeof(logBuffer) / sizeof(char));
  logBufferPos = 0;
}
#endif // USE_XX_LOG

uint16_t stringCrc(const char input[])
{
  uint16_t calcCRC = 0x0000;

  for (unsigned int i = 0; i < strlen(input); i++)
  {
    calcCRC = crc16_update(calcCRC, input[i]);
  }
  return calcCRC;
}

void sendLog()
{
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
  DBG("Logging to buffer");
  for (int i = 0; i < ln; i++)
  {
    StaticJsonDocument<96> doc;
    JsonObject doc_0 = doc.createNestedObject();
    doc_0["id"] = theData[i].id;
    doc_0["type"] = theData[i].t;
    doc_0["data"] = theData[i].d;
    doc_0["time"] = time(nullptr);
    String outgoingString;
    serializeJson(doc, outgoingString);
    outgoingString = outgoingString + " " + stringCrc(outgoingString.c_str()) + "\r\n";
    if (logBufferPos + outgoingString.length() >= (sizeof(logBuffer) / sizeof(char))) // if buffer would overflow, release first
    {
      releaseLogBuffer();
    }
    memcpy(&logBuffer[logBufferPos], outgoingString.c_str(), outgoingString.length()); // append line to buffer
    logBufferPos += outgoingString.length();
  }
  time(&last_log_write);
#endif // USE_xx_LOG
}

void resendLog()
{
#ifdef USE_SD_LOG
  DBG("Resending logged values from SD card.");
  File logfile = SD.open(LOG_FILENAME, FILE_READ);
  while (1)
  {
    String line = logfile.readStringUntil('\n');
    if (line.length() > 0)
    { // if line contains something
      if (!client.publish(TOPIC_DATA_BACKLOG, line.c_str()))
      {
        break;
      }
      else
      {
        time(&last_mqtt_success);
      }
    }
    else
    {
      logfile.close();
      SD.remove(LOG_FILENAME); // if all values are sent
      break;
    }
  }
  DBG(" Done");
#endif
#ifdef USE_FS_LOG
  DBG("Resending logged values from internal flash.");
  File logfile = LittleFS.open(FS_FILENAME, "r");
  while (1)
  {
    String line = logfile.readStringUntil('\n');
    if (line.length() > 0)
    { // if line contains something
      uint16_t readCrc;
      char data[line.length()];
      sscanf(line.c_str(), "%s %hd", data, &readCrc);
      if (stringCrc(data) != readCrc)
      {
        continue;
      } // if CRCs don't match, skip the line
      if (!client.publish(TOPIC_DATA_BACKLOG, line.c_str()))
      {
        break;
      }
      else
      {
        time(&last_mqtt_success);
      }
    }
    else
    {
      logfile.close();
      LittleFS.remove(FS_FILENAME); // if all values are sent
      break;
    }
  }
  DBG(" Done");
#endif
}

void begin_SD()
{
#ifdef USE_SD_LOG
  DBG("Initializing SD card...");
#ifdef ESP32
  SPI.begin(SCK, MISO, MOSI);
#endif
  if (!SD.begin(SD_SS))
  {
    DBG(" Initialization failed!");
    while (1)
      ;
  }
  else
  {
    DBG(" SD initialized.");
  }
#endif // USE_SD_LOG
}

void begin_FS()
{
#ifdef USE_FS_LOG
  DBG("Initializing LittleFS...");

  if (!LittleFS.begin())
  {
    DBG(" initialization failed");
    while (1)
      ;
  }
  else
  {
    DBG(" LittleFS initialized");
  }
#endif // USE_FS_LOG
}