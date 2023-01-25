
#ifdef FDRS_DEBUG
#ifdef USE_OLED
#define DBG(a) debug_OLED(String(a)); \
Serial.println(a);
#else
#define DBG(a) Serial.println(a);
#endif
#else
#ifdef USE_OLED
#define DBG(a) debug_OLED(String(a));
#else
#define DBG(a)
#endif
#endif
