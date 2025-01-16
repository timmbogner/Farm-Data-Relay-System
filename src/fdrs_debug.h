#ifndef DBG_LEVEL
    #define DBG_LEVEL GLOBAL_DBG_LEVEL
#endif

#ifdef FDRS_DEBUG
    #ifdef USE_OLED
        #define DBG(a) Serial.print("    "); Serial.println(a); debug_OLED(String(a));
    #else
        #define DBG(a) Serial.print("    "); Serial.println(a);
    #endif // USE_OLED
    #if (DBG_LEVEL == 0)
        #define DBG1(a);
        #define DBG2(a);
    #endif
    #if (DBG_LEVEL == 1)
        #define DBG1(a) Serial.print("[1] "); Serial.println(a);
        #define DBG2(a)
    #endif
    #if (DBG_LEVEL >= 2)
        #define DBG1(a) Serial.print("[1] "); Serial.println(a);
        #define DBG2(a) Serial.print("[2] "); Serial.println(a);
    #endif
#else
    #ifdef USE_OLED
        #define DBG(a) debug_OLED(String(a));
    #else
        #define DBG(a)
    #endif // USE_OLED
    #define DBG1(a)
    #define DBG2(a)
#endif // FDRS_DEBUG