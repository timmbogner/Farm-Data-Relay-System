//  FARM DATA RELAY SYSTEM
//
//  DETAILED NODES' CONFIGURATION CHECK
//
//  Make sure #define DEBUG_CONFIG is not uncommented in your node's config
// (fdrs_node_config.h or fdrs_gateway_config.h). Otherwise check will be ignored.
//  When the node powers up, it's full config will be printed to the serial console once.
//  Be sure to add further checks as new configuration possibilities are added to FDRS.
//
//  Contributed by Sascha Juch (sascha.juch@gmail.com)
//
#ifndef __FDRS_CHECKCONFIG_h__
#define __FDRS_CHECKCONFIG_h__

const char* separatorLine   = "--------------------------------------------------------------";
const char* headerAndFooter = "==============================================================";


// helper function for obfuscating passwords
String obfuscatePassword(String password) {
	char obfuscatedPass[password.length()];
	// TO DO: The following line is disabled due to AVR incompatibility.
	// std::fill(obfuscatedPass, obfuscatedPass + password.length(), '*'); 
	return String(obfuscatedPass);
}


// helper function for small header above each sub section
void printSmallSectionHeader(const char* headerText) {
	DBG(separatorLine);
	DBG(headerText);
}


// helper function for a nice little header above each section
void printSectionHeader(const char* headerText) {
	DBG(separatorLine);
	DBG(headerText);
	DBG(separatorLine);
}


// helper function for a nice little header above each main section
void printConfigHeader(const char* headerText) {
	DBG(headerAndFooter);
	DBG(headerText);
	DBG(headerAndFooter);
}


// check which logging method(s) have been activated for a node
void printLoggingInformation() {
	printSectionHeader("LOG SETTINGS OF DEVICE");

#if defined(USE_SD_LOG) && defined(USE_FS_LOG)
	DBG("Logging to SD card AND file system is active! You should better use only one of them at a time");
#endif

#ifdef USE_SD_LOG
	DBG("Logging to SD-Card    : enabled");
#ifdef LOGBUF_DELAY
	DBG("log buffer delay in ms: " + String(LOGBUF_DELAY));
#else
	DBG("log buffer delay in ms: NOT SPECIFIED - check config!");
#endif
#ifdef LOG_FILENAME
	DBG("log filename          : " + LOG_FILENAME);
#else
	DBG("log filename          : NOT SPECIFIED - check config!");
#endif
#else
	DBG("Logging to SD-Card    : disabled");
#endif //USE_SD_LOG

#ifdef USE_FS_LOG
	DBG("Logging to file system: enabled");
#ifdef LOGBUF_DELAY
	DBG("log buffer delay in ms: " + String(LOGBUF_DELAY));
#else
	DBG("log buffer delay in ms: NOT SPECIFIED - check config!");
#endif
#ifdef LOG_FILENAME
	DBG("log filename          : " + LOG_FILENAME);
#else
	DBG("log filename          : NOT SPECIFIED - check config!");
#endif
	DBG("WARNING: Permanently logging to flash memory may destroy the flash memory of your device!");
#else
	DBG("Logging to file system: disabled");
#endif //USE_FS_LOG
}


// check which protocols are activated and which are deactivated
void printActivatedProtocols() {
	// current candidates are: ESPNOW, LORA and MQTT (WIFI)
	printSectionHeader("ACTIVATED PROTOCOLS");

#ifdef USE_LORA
	DBG("LoRa   : ENABLED");
#else
	DBG("LoRa   : DISABLED");
#endif

#ifdef USE_ESPNOW
	DBG("ESPNow : ENABLED");
#else
	DBG("ESPNow : DISABLED");
#endif

#ifdef USE_WIFI
	DBG("WiFi   : ENABLED");
#else
	DBG("WiFi   : DISABLED");
#endif

#if defined(USE_WIFI) && defined(USE_ESPNOW)
	DBG("WARNING: You must not use USE_ESPNOW and USE_WIFI together! USE_WIFI is only needed for MQTT!");
#endif

#ifdef USE_STATIC_IPADDRESS
	DBG("Using Static IP Address");
#else
	DBG("Using DHCP");
#endif
}


void printEspnowDetails() {
#ifdef USE_ESPNOW

#ifdef UNIT_MAC
	printSmallSectionHeader("ESP-Now Details:");
	DBG("Neighbor 1 address: " + String(ESPNOW_NEIGHBOR_1, HEX));
	DBG("Neighbor 2 address: " + String(ESPNOW_NEIGHBOR_2, HEX));
#endif //UNIT_MAC

#endif //USE_ESPNOW
}


void printWifiDetails() {
#ifdef USE_WIFI
	printSmallSectionHeader("WiFi Details:");

#if defined(WIFI_SSID)
	DBG("WiFi SSID used from WIFI_SSID            : " + String(FDRS_WIFI_SSID));
#elif defined (GLOBAL_WIFI_SSID)
	DBG("WiFi SSID used from GLOBAL_WIFI_SSID          : " + String(FDRS_WIFI_SSID));
#else 
	DBG("NO WiFi SSID defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //WIFI_SSID

#if defined(WIFI_PASS)
	DBG("WiFi password used from WIFI_PASS        : " + obfuscatePassword(FDRS_WIFI_PASS));
#elif defined (GLOBAL_WIFI_SSID)
	DBG("WiFi password used from GLOBAL_WIFI_PASS      : " + obfuscatePassword(FDRS_WIFI_PASS));
#else 
	DBG("NO WiFi password defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //WIFI_PASS

#ifdef USE_STATIC_IPADDRESS
#if defined(HOST_IPADDRESS)
	DBG("Host IP Address used from HOST_IPADDRESS            : " + String(FDRS_HOST_IPADDRESS));
#elif defined (GLOBAL_HOST_IPADDRESS)
	DBG("Host IP Address used from GLOBAL_HOST_IPADDRESS     : " + String(FDRS_HOST_IPADDRESS));
#else 
	DBG("NO Host IP Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_gateway_config.h");
	//exit(0);
#endif // HOST_IPADDRESS

#if defined(GW_IPADDRESS)
	DBG("Gateway IP Address used from GW_IPADDRESS            : " + String(FDRS_GW_IPADDRESS));
#elif defined (GLOBAL_GW_IPADDRESS)
	DBG("Gateway IP Address used from GLOBAL_GW_IPADDRESS     : " + String(FDRS_GW_IPADDRESS));
#else 
	DBG("NO Gateway IP Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_gateway_config.h");
	//exit(0);
#endif // GW_IPADDRESS

#if defined(SUBNET_ADDRESS)
	DBG("Subnet Address used from SUBNET_ADDRESS            : " + String(FDRS_SUBNET_ADDRESS));
#elif defined (GLOBAL_SUBNET_ADDRESS)
	DBG("Subnet Address used from GLOBAL_SUBNET_ADDRESS     : " + String(FDRS_SUBNET_ADDRESS));
#else 
	DBG("NO Subnet Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_gateway_config.h");
	//exit(0);
#endif // SUBNET_ADDRESS

#if defined(DNS2_IPADDRESS)
	DBG("DNS2 IP Address used from DNS2_IPADDRESS            : " + String(FDRS_DNS2_IPADDRESS));
#elif defined (GLOBAL_DNS2_IPADDRESS)
	DBG("DNS2 IP Address used from GLOBAL_DNS2_IPADDRESS     : " + String(FDRS_DNS2_IPADDRESS));
#endif // DNS2_IPADDRESS
#endif // USE_STATIC_IPADDRESS

#if defined(DNS1_IPADDRESS)
	DBG("DNS1 IP Address used from DNS1_IPADDRESS            : " + String(FDRS_DNS1_IPADDRESS));
#elif defined (GLOBAL_DNS1_IPADDRESS)
	DBG("DNS1 IP Address used from GLOBAL_DNS1_IPADDRESS     : " + String(FDRS_DNS1_IPADDRESS));
#else 
	DBG("NO DNS1 IP Address defined! Please define in fdrs_globals.h (recommended) or in fdrs_gateway_config.h");
	//exit(0);
#endif // DNS1_IPADDRESS


	printSmallSectionHeader("MQTT BROKER CONFIG:");

#if defined(MQTT_ADDR)
	DBG("MQTT address used from MQTT_ADDR         : " + String(FDRS_MQTT_ADDR));
#elif defined (GLOBAL_MQTT_ADDR)
	DBG("MQTT address used from GLOBAL_MQTT_ADDR  : " + String(FDRS_MQTT_ADDR));
#else 
	DBG("NO MQTT address defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
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
	DBG("NO MQTT username defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //MQTT_USER

#if defined(MQTT_PASS)
	DBG("MQTT password used from MQTT_PASS        : " + obfuscatePassword(FDRS_MQTT_PASS));
#elif defined (GLOBAL_MQTT_PASS)
	DBG("MQTT password used from GLOBAL_MQTT_PASS : " + obfuscatePassword(FDRS_MQTT_PASS));
#else 
	DBG("NO MQTT password defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //MQTT_PASS

#endif //FDRS_MQTT_AUTH

#if defined(TOPIC_DATA)
	DBG("MQTT topic (TOPIC_DATA)                  : " + String(TOPIC_DATA));
#else 
	DBG("NO MQTT topic defined! Please define TOPIC_DATA in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //TOPIC_DATA

#if defined(TOPIC_STATUS)
	DBG("MQTT topic (TOPIC_STATUS)                : " + String(TOPIC_STATUS));
#else 
	DBG("NO MQTT topic defined! Please define TOPIC_STATUS in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //TOPIC_STATUS

#if defined(TOPIC_COMMAND)
	DBG("MQTT topic (TOPIC_COMMAND)               : " + String(TOPIC_COMMAND));
#else 
	DBG("NO MQTT topic defined! Please define TOPIC_COMMAND in fdrs_globals.h (recommended) or in fdrs_node_config.h / fdrs_gateway_config.h");
	//exit(0);
#endif //TOPIC_COMMAND

	DBG(separatorLine);
	DBG(separatorLine);

#endif //USE_WIFI
}


void printLoraDetails() {
#ifdef USE_LORA
	printSmallSectionHeader("LoRa Details:");
	
#if defined(FDRS_LORA_FREQUENCY)
	DBG("LoRa frequency used from FDRS_LORA_FREQUENCY                 : " + String(FDRS_LORA_FREQUENCY));
#elif defined (GLOBAL_FDRS_LORA_FREQUENCY)
	DBG("LoRa frequency used from GLOBAL_FDRS_LORA_FREQUENCY          : " + String(FDRS_LORA_FREQUENCY));
#else 
	DBG("NO FDRS_LORA_FREQUENCY defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
	//exit(0);
#endif //LORA-FREQUENCY

#if defined(LORA_SF)
	DBG("LoRa SF used from LORA_SF                     : " + String(FDRS_LORA_SF));
#elif defined (GLOBAL_LORA_SF)
	DBG("LoRa SF used from GLOBAL_LORA_SF              : " + String(FDRS_LORA_SF));
#else 
//	ASSERT("NO LORA-SF defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
	DBG("NO LORA_SF defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
	//exit(0);
#endif //LORA_SF

#if defined(LORA_TXPWR)
	DBG("LoRa TXPWR used from LORA_TXPWR               : " + String(FDRS_LORA_TXPWR));
#elif defined (GLOBAL_LORA_TXPWR)
	DBG("LoRa TXPWR used from GLOBAL_LORA_TXPWR        : " + String(FDRS_LORA_TXPWR));
#else 
//	ASSERT("NO LORA-TXPWR defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
	DBG("NO LORA_TXPWR defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
	//exit(0);
#endif //LORA_TXPWR

#if defined(LORA_ACK)
	DBG("LoRa acknowledgement used from LORA_ACK       : enabled");
#elif defined (GLOBAL_LORA_ACK)
	DBG("LoRa acknowledgement used from GLOBAL_LORA_ACK: enabled");
#else
	DBG("LoRa acknowledgement                          : disabled");
#endif	

#if defined(LORA_ACK) || defined(GLOBAL_LORA_ACK)

#if defined(LORA_ACK_TIMEOUT)
	DBG("Timeout for Lora acknowledment (LORA_ACK)     : " + String(LORA_ACK_TIMEOUT));
#else
	DBG("NO LORA_ACK_TIMEOUT defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // LORA_ACK_TIMEOUT

#if defined(LORA_RETRIES)
	DBG("Number of ack retries (LORA_RETRIES)          : " + String(LORA_RETRIES));
	
	if (LORA_RETRIES >= 0 && LORA_RETRIES <= 3) 
	{
		DBG("Number of ack retries (LORA_RETRIES)          : within allowed range.");
	}
	else {
		DBG("Number of ack retries (LORA_RETRIES)          : not within allowed range [0 - 3]! Please change to correct value.");
	} // LORA_RETRIES RANGE CHECK

#else
	DBG("NO LORA_RETRIES defined! Defaulting to 0. Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // LORA_RETRIES


#endif //LORA_ACK || GLOBAL_LORA_ACK

#ifdef UNIT_MAC
	DBG("LoRa Neighbors");
	DBG("Neighbor 1 address: " + String(LORA_NEIGHBOR_1, HEX));
	DBG("Neighbor 2 address: " + String(LORA_NEIGHBOR_2, HEX));
#endif //UNIT_MAC

#endif //USE_LORA
}


void checkConfig() {
	printConfigHeader("NODE CONFIGURATION OVERVIEW");
#ifdef UNIT_MAC
	DBG("Device Type       : Gateway");
	DBG("Gateway ID      : " + String(UNIT_MAC, HEX));
#elif defined (READING_ID)
	DBG("Device Type       : Node");
	DBG("Reading ID      : " + String(READING_ID));
	DBG("Node's Gateway: " + String(GTWY_MAC, HEX));
#else
	DBG("Device Type       : UNKNOWN!");
	DBG("Please check config!");
	DBG("If you have just created a new node type,");
	DBG("please add it's config check to:");
	DGB("fdrs_checkConfig.h");
#endif
	
	printActivatedProtocols();
	
	printSmallSectionHeader("PROTOCOL DETAILS");

#ifdef USE_LORA
	printLoraDetails();
#endif

#ifdef USE_ESPNOW
	printEspnowDetails();
#endif

#ifdef USE_WIFI
	printWifiDetails();
#endif

	printLoggingInformation();
	
	printConfigHeader("NODE CONFIGURATION OVERVIEW END");
	//DBG(separatorLine);
	DBG("");
}

#endif //__FDRS_CHECKCONFIG_h__

