#include <PubSubClient.h>
#include <ArduinoJson.h>

// select MQTT server address
#if defined(MQTT_ADDR)
#define FDRS_MQTT_ADDR MQTT_ADDR
#elif defined(GLOBAL_MQTT_ADDR)
#define FDRS_MQTT_ADDR GLOBAL_MQTT_ADDR
#else
// ASSERT("NO MQTT address defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // MQTT_ADDR

// select MQTT server port
#if defined(MQTT_PORT)
#define FDRS_MQTT_PORT MQTT_PORT
#elif defined(GLOBAL_MQTT_PORT)
#define FDRS_MQTT_PORT GLOBAL_MQTT_PORT
#else
#define FDRS_MQTT_PORT 1883
#endif // MQTT_PORT

// select MQTT user name
#if defined(MQTT_USER)
#define FDRS_MQTT_USER MQTT_USER
#elif defined(GLOBAL_MQTT_USER)
#define FDRS_MQTT_USER GLOBAL_MQTT_USER
#else
// ASSERT("NO MQTT user defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // MQTT_USER

// select MQTT user password
#if defined(MQTT_PASS)
#define FDRS_MQTT_PASS MQTT_PASS
#elif defined(GLOBAL_MQTT_PASS)
#define FDRS_MQTT_PASS GLOBAL_MQTT_PASS
#else
// ASSERT("NO MQTT password defined! Please define in fdrs_globals.h (recommended) or in fdrs_node_config.h");
#endif // MQTT_PASS
#if defined(MQTT_AUTH) || defined(GLOBAL_MQTT_AUTH)
#define FDRS_MQTT_AUTH
#endif // MQTT_AUTH


WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = FDRS_MQTT_ADDR;
const int mqtt_port = FDRS_MQTT_PORT;
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
    extern time_t last_log_write;
    extern time_t last_mqtt_success;
#endif


#ifdef FDRS_MQTT_AUTH
const char *mqtt_user = FDRS_MQTT_USER;
const char *mqtt_pass = FDRS_MQTT_PASS;
#else
const char *mqtt_user = NULL;
const char *mqtt_pass = NULL;
#endif // FDRS_MQTT_AUTH

void reconnect_mqtt(short int attempts, bool silent)
{
    if (!silent)
        DBG("Connecting MQTT...");

    for (short int i = 1; i <= attempts; i++)
    {
        // Attempt to connect
        if (client.connect("FDRS_GATEWAY", mqtt_user, mqtt_pass))
        {
            // Subscribe
            client.subscribe(TOPIC_COMMAND);
            if (!silent)
                DBG(" MQTT Connected");
            return;
        }
        else
        {
            if (!silent)
            {
                char msg[23];
                sprintf(msg, " Attempt %d/%d", i, attempts);
                DBG(msg);
            }
            if ((attempts != 1))
            {
                delay(3000);
            }
        }
    }

    if (!silent)
        DBG(" Connecting MQTT failed.");
}

void reconnect_mqtt(int attempts)
{
    reconnect_mqtt(attempts, false);
}

// Handles MQTT in loop()
void handleMQTT()
{
    if (!client.connected())
    {
        reconnect_mqtt(1, true);
    }
    client.loop(); // for recieving incoming messages and maintaining connection
}

void mqtt_callback(char *topic, byte *message, unsigned int length)
{
    String incomingString;
    DBG(topic);
    for (unsigned int i = 0; i < length; i++)
    {
        incomingString += (char)message[i];
    }
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, incomingString);
    if (error)
    { // Test if parsing succeeds.
        DBG("json parse err");
        DBG(incomingString);
        return;
    }
    else
    {
        int s = doc.size();
        // UART_IF.println(s);
        for (int i = 0; i < s; i++)
        {
            theData[i].id = doc[i]["id"];
            theData[i].t = doc[i]["type"];
            theData[i].d = doc[i]["data"];
        }
        ln = s;
        newData = event_mqtt;
        DBG("Incoming MQTT.");
    }
}

void begin_mqtt()
{
    client.setServer(mqtt_server, mqtt_port);
    if (!client.connected())
    {
        reconnect_mqtt(5);
    }
    client.setCallback(mqtt_callback);
}

void mqtt_publish(const char *payload)
{
    if (!client.publish(TOPIC_DATA, payload))
    {
        DBG(" Error on sending MQTT");
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
        sendLog();
#endif
    }
    else
    {
#if defined(USE_SD_LOG) || defined(USE_FS_LOG)
        if (last_log_write >= last_mqtt_success)
        {
            releaseLogBuffer();
            resendLog();
        }
        time(&last_mqtt_success);
#endif
    }
}

void sendMQTT()
{
    DBG("Sending MQTT.");
    DynamicJsonDocument doc(24576);
    for (int i = 0; i < ln; i++)
    {
        doc[i]["id"] = theData[i].id;
        doc[i]["type"] = theData[i].t;
        doc[i]["data"] = theData[i].d;
        doc[i]["time"] = time(nullptr);
    }
    String outgoingString;
    serializeJson(doc, outgoingString);
    mqtt_publish((char *)outgoingString.c_str());
}