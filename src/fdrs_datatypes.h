// A list of all datatypes you can use within FDRS.
// If you are missing any data type, please open an issue at:
// https://github.com/timmbogner/Farm-Data-Relay-System/issues
 
typedef struct FDRSPeer {
  uint8_t mac[6];
  uint32_t last_seen = 0;

} FDRSPeer;

typedef struct __attribute__((packed)) DataReading {
  float d;
  uint16_t id;
  uint8_t t;

} DataReading;

typedef struct __attribute__((packed)) SystemPacket {
  uint8_t cmd;
  uint32_t param;
} SystemPacket;

enum crcResult {
  CRC_NULL,
  CRC_OK,
  CRC_BAD,
} returnCRC;

enum cmd_t {
  cmd_clear,
  cmd_ping,
  cmd_add,
  cmd_ack,
  cmd_time
};

enum ping_t {
  ping_request,
  ping_reply
};

enum commstate_t {
  stReady,
  stInProcess,
  stCrcMismatch,
  stCrcMatch,
  stInterMessageDelay,
  stCompleted
};


enum
{
  event_clear,
  event_espnowg,
  event_espnow1,
  event_espnow2,
  event_serial,
  event_mqtt,
  event_lorag,
  event_lora1,
  event_lora2,
  event_internal
};

// Interface type that is the time source
enum TmNetIf {
  TMIF_NONE,
  TMIF_LORA,
  TMIF_ESPNOW,
  TMIF_SERIAL,
  TMIF_LOCAL,
};
// Local time source that is setting the time
enum TmSource {
  TMS_NONE,
  TMS_NET,
  TMS_RTC,
  TMS_NTP,
  TMS_GPS,
};

struct TimeSource {
  TmNetIf tmNetIf;
  uint16_t tmAddress;
  TmSource tmSource;
  unsigned long tmLastTimeSet;
};

struct DRRingBuffer {
DataReading *dr;
uint16_t *address;
uint startIdx;
uint endIdx;
uint size;
};

struct SPRingBuffer {
SystemPacket *sp;
uint16_t *address;
uint startIdx;
uint endIdx;
uint size;
};

struct Ping {
  commstate_t status = stReady;
  unsigned long start;
  uint timeout;
  uint16_t address;
  uint32_t response = __UINT32_MAX__;
};

#ifndef ESP32
typedef int esp_err_t;
#define ESP_FAIL 0
#define ESP_OK 1
#endif

#ifndef FDRS_DATA_TYPES
#define FDRS_DATA_TYPES

#define STATUS_T        0  // Status 
#define TEMP_T          1  // Temperature 
#define TEMP2_T         2  // Temperature #2
#define HUMIDITY_T      3  // Relative Humidity 
#define PRESSURE_T      4  // Atmospheric Pressure 
#define LIGHT_T         5  // Light (lux) 
#define SOIL_T          6  // Soil Moisture 
#define SOIL2_T         7  // Soil Moisture #2 
#define SOILR_T         8  // Soil Resistance 
#define SOILR2_T        9  // Soil Resistance #2 
#define OXYGEN_T        10 // Oxygen 
#define CO2_T           11 // Carbon Dioxide
#define WINDSPD_T       12 // Wind Speed
#define WINDHDG_T       13 // Wind Direction
#define RAINFALL_T      14 // Rainfall
#define MOTION_T        15 // Motion
#define VOLTAGE_T       16 // Voltage
#define VOLTAGE2_T      17 // Voltage #2
#define CURRENT_T       18 // Current
#define CURRENT2_T      19 // Current #2
#define IT_T            20 // Iterations
#define LATITUDE_T      21 // GPS Latitude
#define LONGITUDE_T     22 // GPS Longitude
#define ALTITUDE_T      23 // GPS Altitude
#define HDOP_T          24 // GPS HDOP
#define LEVEL_T         25 // Fluid Level
#define UV_T            26 // UV
#define PM1_T           27 // 1 Particles
#define PM2_5_T         28 // 2.5 Particles
#define PM10_T          29 // 10 Particles
#define POWER_T         30 // Power
#define POWER2_T        31 // Power #2
#define ENERGY_T        32 // Energy
#define ENERGY2_T       33 // Energy #2
#define WEIGHT_T        34 // Weight
#define WEIGHT2_T       35 // Weight #2

#endif //FDRS_DATA_TYPES