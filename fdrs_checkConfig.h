
#ifndef __FDRS_CHECKCONFIG_h__
#define __FDRS_CHECKCONFIG_h__

char* separatorLine2 = "----------------------------------------------------";

// helper function for a nice little header above each section
void printSmallSectionHeader(char* headerText) {
	char * separatorLine   = "----------------------------------------------------";

	DBG(separatorLine);
	DBG(headerText);
	//DBG(separatorLine);
}

// helper function for a nice little header above each section
void printSectionHeader(char* headerText) {
	char * separatorLine   = "----------------------------------------------------";

	DBG(separatorLine);
	DBG(headerText);
	DBG(separatorLine);
}

// helper function for a nice little header above each section
void printConfigHeader(char* headerText) {
	char * headerAndFooter = "====================================================";

	DBG(headerAndFooter);
	DBG(headerText);
	DBG(headerAndFooter);
}

// check which protocols are activated and which are deactivated
void printActivatedProtocols() {
	// current candidates are: WIFI, ESPNOW, LORA, MQTT, ???
	printSectionHeader("ACTIVATED PROTOCOLS");

#ifdef USE_LORA
	DBG("LoRa  : ENABLED");
#else
	DBG("LoRa  : DISABLED");
#endif

#ifdef USE_ESPNOW
	DBG("ESPNow: ENABLED");
#else
	DBG("ESPNow: DISABLED");
#endif

#ifdef USE_WIFI
	DBG("WiFi  : ENABLED");
#else
	DBG("WiFi  : DISABLED");
#endif

}


void printEspnowDetails() {
#ifdef USE_ESPNOW

#ifdef UNIT_MAC
	printSmallSectionHeader("ESP-Now Details:");
	DBG("Peer 1 address: " + String(ESPNOW1_PEER, HEX));
	DBG("Peer 2 address: " + String(ESPNOW2_PEER, HEX));
#endif //UNIT_MAC

#endif //USE_ESPNOW
}

void printWifiDetails() {
#ifdef USE_WIFI
	printSmallSectionHeader("WiFi Details:");

#if defined(WIFI_SSID)
	DBG("WiFi SSID used from WIFI_SSID            : " + String(FDRS_WIFI_SSID));
#elif defined (GLOBAL_SSID)
	DBG("WiFi SSID used from GLOBAL_SSID          : " + String(FDRS_WIFI_SSID));
#else 
	DBG("NO WiFi SSID defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //WIFI_SSID

#if defined(WIFI_PASS)
	DBG("WiFi password used from WIFI_PASS        : " + String(FDRS_WIFI_PASS));
#elif defined (GLOBAL_SSID)
	DBG("WiFi password used from GLOBAL_PASS      : " + String(FDRS_WIFI_PASS));
#else 
	DBG("NO WiFi password defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //WIFI_PASS

	printSmallSectionHeader("MQTT BROKER CONFIG:");

#if defined(MQTT_ADDR)
	DBG("MQTT address used from MQTT_ADDR         : " + String(FDRS_MQTT_ADDR));
#elif defined (GLOBAL_MQTT_ADDR)
	DBG("MQTT address used from GLOBAL_MQTT_ADDR  : " + String(FDRS_MQTT_ADDR));
#else 
	DBG("NO MQTT address defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //MQTT_ADDR

#if defined(MQTT_PORT)
	DBG("MQTT port used from MQTT_PORT            : " + String(FDRS_MQTT_PORT));
#elif defined (GLOBAL_MQTT_PORT)
	DBG("MQTT port used from GLOBAL_MQTT_ADDR     : " + String(FDRS_MQTT_PORT));
#else 
	DBG("Using default MQTT port                  : " + String(FDRS_MQTT_PORT));
#endif //MQTT_PORT

#ifdef FDRS_MQTT_AUTH
	printSmallSectionHeader("MQTT AUTHENTIFICATION CONFIG:");
//GLOBAL_MQTT_AUTH
#if defined(MQTT_USER)
	DBG("MQTT username used from MQTT_USER        : " + String(FDRS_MQTT_USER));
#elif defined (GLOBAL_MQTT_USER)
	DBG("MQTT username used from GLOBAL_MQTT_USER : " + String(FDRS_MQTT_USER));
#else 
	DBG("NO MQTT username defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //MQTT_USER

#if defined(MQTT_PASS)
	DBG("MQTT password used from MQTT_PASS        : " + String(FDRS_MQTT_PASS));
#elif defined (GLOBAL_MQTT_PASS)
	DBG("MQTT password used from GLOBAL_MQTT_PASS : " + String(FDRS_MQTT_PASS));
#else 
	DBG("NO MQTT password defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //MQTT_PASS

#endif //FDRS_MQTT_AUTH
	DBG("----------------------------------------------------");
	DBG(separatorLine2);

#endif //USE_WIFI


}

void printLoraDetails() {
#ifdef USE_LORA
	printSmallSectionHeader("LoRa Details:");
	
#if defined(LORA_BAND)
	DBG("LoRa Band used from LORA_BAND       : " + String(FDRS_BAND));
#elif defined (GLOBAL_LORA_BAND)
	DBG("LoRa Band used from GLOBAL_LORA_BAND: " + String(FDRS_BAND));
#else 
	DBG("NO LORA-BAND defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //LORA-BAND

#if defined(LORA_SF)
	DBG("LoRa SF used from LORA_SF           : " + String(FDRS_SF));
#elif defined (GLOBAL_LORA_SF)
	DBG("LoRa SF used from GLOBAL_LORA_SF    : " + String(FDRS_SF));
#else 
//	ASSERT("NO LORA-SF defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	DBG("NO LORA-SF defined! Please define in fdrs_globals.h (recommended) or in fdrs_sensor_config.h");
	//exit(0);
#endif //LORA-SF

#ifdef UNIT_MAC
	DBG("LoRa peers");
	DBG("Peer 1 address: " + String(LORA1_PEER, HEX));
	DBG("Peer 2 address: " + String(LORA2_PEER, HEX));
#endif //UNIT_MAC

#endif //USE_LORA
}


void checkConfig() {
	printConfigHeader("NODE CONFIGURATION OVERVIEW");
#ifdef UNIT_MAC
	DBG("Node Type       : Gateway");
	DBG("Gateway ID      : " + String(UNIT_MAC, HEX));
#elif defined (READING_ID)
	DBG("Node Type       : Sensor");
	DBG("Reading ID      : " + String(READING_ID));
	DBG("Sensor's Gateway: " + String(GTWY_MAC, HEX));
#else
	DBG("Node Type       : UNKNOWN!");
	DBG("Please check config!");
	DBG("If you have just created a new node type,");
	DBG("please add it's config check to:");
	DGB("fdrs_checkConfig.h");
#endif

	//printConfigHeader("FULL CONFIG OVERVIEW");
	
	printActivatedProtocols();
	
	printSmallSectionHeader("PROTOCOL DETAILS");

#ifdef USE_LORA
	printLoraDetails();
#endif

// why is USE_ESPNOW not defined for gateways? This should be implemented as not every gateway has to be an ESP-NOW gateway!
#ifdef USE_ESPNOW
	printEspnowDetails();
#endif

#ifdef USE_WIFI
	printWifiDetails();
#endif



	DBG("----------------------------------------------------");
	DBG("");
}

#endif //__FDRS_CHECKCONFIG_h__

