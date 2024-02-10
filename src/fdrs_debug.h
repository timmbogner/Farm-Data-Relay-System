
#if !defined(FDRS_DEBUG) && !defined(FDRS_DEBUG_1) && !defined(FDRS_DEBUG_2)
#ifdef USE_OLED
#define DBG(a) debug_OLED(String(a));
#define DBG1(a)
#define DBG2(a)
#else
#define DBG(a)
#define DBG1(a)
#define DBG2(a)
#endif
#endif

#ifdef FDRS_DEBUG
#ifdef USE_OLED
#define DBG(a) Serial.println(a); debug_OLED(String(a));
#else
#define DBG(a) Serial.println(a);
#endif // USE_OLED
#define DBG1(a)
#define DBG2(a)
#endif // FDRS_DEBUG

#ifdef FDRS_DEBUG_1
#ifdef USE_OLED
#define DBG(a) Serial.print("    "); Serial.println(a); debug_OLED(String(a));
#else
#define DBG(a) Serial.print("    "); Serial.println(a);
#endif // USE_OLED
#define DBG1(a) Serial.print("[1] "); Serial.println(a);
#define DBG2(a)
#endif //FDRS_DEBUG_1

#ifdef FDRS_DEBUG_2
#ifdef USE_OLED
#define DBG(a) Serial.print("    "); Serial.println(a); debug_OLED(String(a));
#else
#define DBG(a) Serial.print("    "); Serial.println(a);
#endif // USE_OLED
#define DBG2(a) Serial.print("[2] "); Serial.println(a);
#define DBG1(a) Serial.print("[1] "); Serial.println(a);
#endif //FDRS_DEBUG_2