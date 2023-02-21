#ifdef USE_LORA
#include <ArduinoUniqueID.h>
#include <RadioLib.h>
#endif

// Internal Globals
// Default values: overridden by settings in config, if present

#define GLOBAL_ACK_TIMEOUT 400 // LoRa ACK timeout in ms. (Minimum = 200)
#define GLOBAL_LORA_RETRIES 2  // LoRa ACK automatic retries [0 - 3]
#define GLOBAL_LORA_TXPWR 17   // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))

#ifdef USE_LORA
// select LoRa band configuration
#if defined(LORA_FREQUENCY)
#define FDRS_LORA_FREQUENCY LORA_FREQUENCY
#else
#define FDRS_LORA_FREQUENCY GLOBAL_LORA_FREQUENCY
#endif // LORA_FREQUENCY

// select LoRa SF configuration
#if defined(LORA_SF)
#define FDRS_LORA_SF LORA_SF
#else
#define FDRS_LORA_SF GLOBAL_LORA_SF
#endif // LORA_SF

// select LoRa ACK configuration
#if defined(LORA_ACK) || defined(GLOBAL_LORA_ACK)
#define FDRS_LORA_ACK
#endif // LORA_ACK

// select LoRa ACK Timeout configuration
#if defined(LORA_ACK_TIMEOUT)
#define FDRS_ACK_TIMEOUT LORA_ACK_TIMEOUT
#else
#define FDRS_ACK_TIMEOUT GLOBAL_ACK_TIMEOUT
#endif // LORA_ACK_TIMEOUT

// select LoRa Retry configuration
#if defined(LORA_RETRIES)
#define FDRS_LORA_RETRIES LORA_RETRIES
#else
#define FDRS_LORA_RETRIES GLOBAL_LORA_RETRIES
#endif // LORA_RETRIES

// select  LoRa Tx Power configuration
#if defined(LORA_TXPWR)
#define FDRS_LORA_TXPWR LORA_TXPWR
#else
#define FDRS_LORA_TXPWR GLOBAL_LORA_TXPWR
#endif // LORA_TXPWR

// select  LoRa BANDWIDTH configuration
#if defined(LORA_BANDWIDTH)
#define FDRS_LORA_BANDWIDTH LORA_BANDWIDTH
#else
#define FDRS_LORA_BANDWIDTH GLOBAL_LORA_BANDWIDTH
#endif // LORA_BANDWIDTH

// select  LoRa Coding Rate configuration
#if defined(LORA_CR)
#define FDRS_LORA_CR LORA_CR
#else
#define FDRS_LORA_CR GLOBAL_LORA_CR
#endif // LORA_CR

// select  LoRa SyncWord configuration
#if defined(LORA_SYNCWORD)
#define FDRS_LORA_SYNCWORD LORA_SYNCWORD
#else
#define FDRS_LORA_SYNCWORD GLOBAL_LORA_SYNCWORD
#endif // LORA_SYNCWORD

#ifdef CUSTOM_SPI
#ifdef ESP32
SPIClass LORA_SPI(HSPI);
RADIOLIB_MODULE radio = new Module(LORA_SS, LORA_DIO, LORA_RST, -1, LORA_SPI);
#endif // ESP32
#else
RADIOLIB_MODULE radio = new Module(LORA_SS, LORA_DIO, LORA_RST, -1);
#endif                                // CUSTOM_SPI
bool transmitFlag = false;            // flag to indicate transmission or reception state
volatile bool enableInterrupt = true; // disable interrupt when it's not needed
volatile bool operationDone = false;  // flag to indicate that a packet was sent or received

unsigned long receivedLoRaMsg = 0; // Number of total LoRa packets destined for us and of valid size
unsigned long ackOkLoRaMsg = 0;    // Number of total LoRa packets with valid CRC

uint16_t LoRaAddress = ((UniqueID8[6] << 8) | UniqueID8[7]);

unsigned long transmitLoRaMsgwAck = 0; // Number of total LoRa packets destined for us and of valid size
unsigned long msgOkLoRa = 0;           // Number of total LoRa packets with valid CRC
void printLoraPacket(uint8_t *p, int size);

uint16_t gtwyAddress = ((gatewayAddress[4] << 8) | GTWY_MAC);
crcResult getLoRa();

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void)
{
    if (!enableInterrupt)
    { // check if the interrupt is enabled
        return;
    }
    operationDone = true; // we sent or received  packet, set the flag
}
#endif // USE_LORA

void handleLoRa()
{
#ifdef USE_LORA
    if (operationDone)
    { // the interrupt was triggered
        enableInterrupt = false;
        operationDone = false;
        if (transmitFlag)
        {                         // the previous operation was transmission,
            radio.startReceive(); // return to listen mode
            enableInterrupt = true;
            transmitFlag = false;
        }
        else
        { // the previous operation was reception
            returnCRC = getLoRa();
            if (!transmitFlag) // return to listen if no transmission was begun
            {
                radio.startReceive();
            }
            enableInterrupt = true;
        }
    }
#endif // USE_LORA
}

void begin_lora()
{

#ifdef USE_LORA
#ifdef CUSTOM_SPI
#ifdef ESP32
    LORA_SPI.begin(LORA_SPI_SCK, LORA_SPI_MISO, LORA_SPI_MOSI);
#endif // ESP32
#else
#endif // CUSTOM_SPI

    int state = radio.begin(FDRS_LORA_FREQUENCY, FDRS_LORA_BANDWIDTH, FDRS_LORA_SF, FDRS_LORA_CR, FDRS_LORA_SYNCWORD, FDRS_LORA_TXPWR, 8, 0);
    if (state == RADIOLIB_ERR_NONE)
    {
        DBG("RadioLib initialization successful!");
    }
    else
    {
        DBG("RadioLib initialization failed, code " + String(state));
        while (true)
            ;
    }
    DBG("LoRa Initialized. Frequency: " + String(FDRS_LORA_FREQUENCY) + "  Bandwidth: " + String(FDRS_LORA_BANDWIDTH) + "  SF: " + String(FDRS_LORA_SF) + "  CR: " + String(FDRS_LORA_CR) + "  SyncWord: " + String(FDRS_LORA_SYNCWORD) + "  Tx Power: " + String(FDRS_LORA_TXPWR) + "dBm");
#ifdef USE_SX126X
    radio.setDio1Action(setFlag);
#else
    radio.setDio0Action(setFlag);
#endif
    radio.setCRC(false);
    LoRaAddress = ((radio.randomByte() << 8) | radio.randomByte());
    state = radio.startReceive(); // start listening for LoRa packets
    if (state == RADIOLIB_ERR_NONE)
    {
    }
    else
    {
        DBG(" failed, code " + String(state));
        while (true)
            ;
    }
#endif // USE_LORA
}

void transmitLoRa(uint16_t *destMAC, DataReading *packet, uint8_t len)
{
#ifdef USE_LORA
    uint8_t pkt[6 + (len * sizeof(DataReading))];
    uint16_t calcCRC = 0x0000;

    pkt[0] = (*destMAC >> 8);
    pkt[1] = (*destMAC & 0x00FF);
    pkt[2] = (LoRaAddress >> 8);
    pkt[3] = (LoRaAddress & 0x00FF);
    memcpy(&pkt[4], packet, len * sizeof(DataReading));
    for (int i = 0; i < (sizeof(pkt) - 2); i++)
    { // Last 2 bytes are CRC so do not include them in the calculation itself
        // printf("CRC: %02X : %d\n",calcCRC, i);
        calcCRC = crc16_update(calcCRC, pkt[i]);
    }
#ifndef LORA_ACK
    calcCRC = crc16_update(calcCRC, 0xA1); // Recalculate CRC for No ACK
#endif                                     // LORA_ACK
    pkt[len * sizeof(DataReading) + 4] = (calcCRC >> 8);
    pkt[len * sizeof(DataReading) + 5] = (calcCRC & 0x00FF);
#ifdef LORA_ACK // Wait for ACK
    int retries = FDRS_LORA_RETRIES + 1;
    while (retries != 0)
    {
        if (transmitLoRaMsgwAck != 0)
            DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, HEX) + " to gateway 0x" + String(*destMAC, HEX) + ". Retries remaining: " + String(retries - 1) + ", Ack Ok " + String((float)msgOkLoRa / transmitLoRaMsgwAck * 100) + "%");
        else
            DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, HEX) + " to gateway 0x" + String(*destMAC, HEX) + ". Retries remaining: " + String(retries - 1));
        // printLoraPacket(pkt,sizeof(pkt));
        int state = radio.transmit(pkt, sizeof(pkt));
        transmitFlag = true;
        if (state == RADIOLIB_ERR_NONE)
        {
        }
        else
        {
            DBG(" failed, code " + String(state));
            while (true)
                ;
        }
        transmitLoRaMsgwAck++;
        unsigned long loraAckTimeout = millis() + FDRS_ACK_TIMEOUT;
        retries--;
        delay(10);
        while (returnCRC == CRC_NULL && (millis() < loraAckTimeout))
        {
            handleLoRa();
        }
        if (returnCRC == CRC_OK)
        {
            // DBG("LoRa ACK Received! CRC OK");
            msgOkLoRa++;
            return; // we're done
        }
        else if (returnCRC == CRC_BAD)
        {
            // DBG("LoRa ACK Received! CRC BAD");
            //  Resend original packet again if retries are available
        }
        else
        {
            DBG("LoRa Timeout waiting for ACK!");
            // resend original packet again if retries are available
        }
    }
#else  // Send and do not wait for ACK reply
    DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, HEX) + " to gateway 0x" + String(*destMAC, HEX));
    // printLoraPacket(pkt,sizeof(pkt));
    int state = radio.transmit(pkt, sizeof(pkt));
    transmitFlag = true;
    if (state == RADIOLIB_ERR_NONE)
    {
    }
    else
    {
        DBG(" failed, code " + String(state));
        while (true)
            ;
    }
    transmitLoRaMsgwAck++;
#endif // LORA_ACK
#endif // USE_LORA
}

// For now SystemPackets will not use ACK but will calculate CRC
void transmitLoRa(uint16_t *destMAC, SystemPacket *packet, uint8_t len)
{
#ifdef USE_LORA
    uint8_t pkt[6 + (len * sizeof(SystemPacket))];
    uint16_t calcCRC = 0x0000;

    pkt[0] = (*destMAC >> 8);
    pkt[1] = (*destMAC & 0x00FF);
    pkt[2] = (LoRaAddress >> 8);
    pkt[3] = (LoRaAddress & 0x00FF);
    memcpy(&pkt[4], packet, len * sizeof(SystemPacket));
    for (int i = 0; i < (sizeof(pkt) - 2); i++)
    { // Last 2 bytes are CRC so do not include them in the calculation itself
        // printf("CRC: %02X : %d\n",calcCRC, i);
        calcCRC = crc16_update(calcCRC, pkt[i]);
    }
    calcCRC = crc16_update(calcCRC, 0xA1); // Recalculate CRC for No ACK
    pkt[len * sizeof(SystemPacket) + 4] = (calcCRC >> 8);
    pkt[len * sizeof(SystemPacket) + 5] = (calcCRC & 0x00FF);
    DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, HEX) + " to destination 0x" + String(*destMAC, HEX));
    // printLoraPacket(pkt,sizeof(pkt));
    int state = radio.startTransmit(pkt, sizeof(pkt));
    transmitFlag = true;
    if (state == RADIOLIB_ERR_NONE)
    {
    }
    else
    {
        DBG(" failed, code " + String(state));
        while (true)
            ;
    }
#endif // USE_LORA
}

// getLoRa for Sensors
//  USED to get ACKs (SystemPacket type) from LoRa gateway at this point.  May be used in the future to get other data
// Return type is crcResult struct - CRC_OK, CRC_BAD, CRC_NULL.  CRC_NULL used for non-ack data

crcResult getLoRa()
{
#ifdef USE_LORA
    int packetSize = radio.getPacketLength();
    if ((((packetSize - 6) % sizeof(DataReading) == 0) || ((packetSize - 6) % sizeof(SystemPacket) == 0)) && packetSize > 0)
    { // packet size should be 6 bytes plus multiple of size of DataReading
        uint8_t packet[packetSize];
        uint16_t packetCRC = 0x0000; // CRC Extracted from received LoRa packet
        uint16_t calcCRC = 0x0000;   // CRC calculated from received LoRa packet
        uint16_t sourceMAC = 0x0000;
        uint16_t destMAC = 0x0000;

        radio.readData((uint8_t *)&packet, packetSize);

        destMAC = (packet[0] << 8) | packet[1];
        sourceMAC = (packet[2] << 8) | packet[3];
        packetCRC = ((packet[packetSize - 2] << 8) | packet[packetSize - 1]);
        // DBG("Packet Address: 0x" + String(packet[0], HEX) + String(packet[1], HEX) + " Self Address: 0x" + String(selfAddress[4], HEX) + String(selfAddress[5], HEX));
        if ((destMAC == LoRaAddress) || (destMAC == 0xFFFF))
        { // Check if addressed to this device or broadcast
            // printLoraPacket(packet,sizeof(packet));
            if (receivedLoRaMsg != 0)
            { // Avoid divide by 0
                DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(radio.getRSSI()) + "dBm, SNR: " + String(radio.getSNR()) + "dB, PacketCRC: 0x" + String(packetCRC, HEX) + ", Total LoRa received: " + String(receivedLoRaMsg) + ", CRC Ok Pct " + String((float)ackOkLoRaMsg / receivedLoRaMsg * 100) + "%");
            }
            else
            {
                DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(radio.getRSSI()) + "dBm, SNR: " + String(radio.getSNR()) + "dB, PacketCRC: 0x" + String(packetCRC, HEX) + ", Total LoRa received: " + String(receivedLoRaMsg));
            }
            receivedLoRaMsg++;
            // Evaluate CRC
            for (int i = 0; i < (packetSize - 2); i++)
            { // Last 2 bytes of packet are the CRC so do not include them in calculation
                // printf("CRC: %02X : %d\n",calcCRC, i);
                calcCRC = crc16_update(calcCRC, packet[i]);
            }
            if ((packetSize - 6) % sizeof(DataReading) == 0)
            { // DataReading type packet
                if (calcCRC == packetCRC)
                {
                    SystemPacket ACK = {.cmd = cmd_ack, .param = CRC_OK};
                    DBG("CRC Match, sending ACK packet to node 0x" + String(sourceMAC, HEX) + "(hex)");
                    transmitLoRa(&sourceMAC, &ACK, 1); // Send ACK back to source
                }
                else if (packetCRC == crc16_update(calcCRC, 0xA1))
                { // Sender does not want ACK and CRC is valid
                    DBG("Node address 0x" + String(sourceMAC, 16) + "(hex) does not want ACK");
                }
                else
                {
                    SystemPacket NAK = {.cmd = cmd_ack, .param = CRC_BAD};
                    // Send NAK packet to sensor
                    DBG("CRC Mismatch! Packet CRC is 0x" + String(packetCRC, HEX) + ", Calculated CRC is 0x" + String(calcCRC, HEX) + " Sending NAK packet to node 0x" + String(sourceMAC, HEX) + "(hex)");
                    transmitLoRa(&sourceMAC, &NAK, 1); // CRC did not match so send NAK to source
                    return CRC_BAD;                    // Exit function and do not update newData to send invalid data further on
                }
                memcpy(&theData, &packet[4], packetSize - 6); // Split off data portion of packet (N - 6 bytes (6 bytes for headers and CRC))
                ln = (packetSize - 6) / sizeof(DataReading);
                newData = true;
                ackOkLoRaMsg++;
                return CRC_OK;
            }
            else if ((packetSize - 6) == sizeof(SystemPacket))
            {
                uint ln = (packetSize - 6) / sizeof(SystemPacket);
                SystemPacket receiveData[ln];

                if (calcCRC == packetCRC)
                {
                    memcpy(receiveData, &packet[4], packetSize - 6); // Split off data portion of packet (N bytes)
                    if (ln == 1 && receiveData[0].cmd == cmd_ack)
                    {
                        DBG("ACK Received - CRC Match");
                    }
                    else if (ln == 1 && receiveData[0].cmd == cmd_ping)
                    { // We have received a ping request or reply??
                        if (receiveData[0].param == 1)
                        { // This is a reply to our ping request
                            is_ping = true;
                            DBG("We have received a ping reply via LoRa from address " + String(sourceMAC, HEX));
                        }
                        else if (receiveData[0].param == 0)
                        {
                            DBG("We have received a ping request from 0x" + String(sourceMAC, HEX) + ", Replying.");
                            SystemPacket pingReply = {.cmd = cmd_ping, .param = 1};
                            transmitLoRa(&sourceMAC, &pingReply, 1);
                        }
                    }
                    else
                    { // data we have received is not yet programmed.  How we handle is future enhancement.
                        DBG("Received some LoRa SystemPacket data that is not yet handled.  To be handled in future enhancement.");
                        DBG("ln: " + String(ln) + "data type: " + String(receiveData[0].cmd));
                    }
                    ackOkLoRaMsg++;
                    return CRC_OK;
                }
                else if (packetCRC == crc16_update(calcCRC, 0xA1))
                {                                                    // Sender does not want ACK and CRC is valid
                    memcpy(receiveData, &packet[4], packetSize - 6); // Split off data portion of packet (N bytes)
                    if (ln == 1 && receiveData[0].cmd == cmd_ack)
                    {
                        DBG("ACK Received - CRC Match");
                    }
                    else if (ln == 1 && receiveData[0].cmd == cmd_ping)
                    { // We have received a ping request or reply??
                        if (receiveData[0].param == 1)
                        { // This is a reply to our ping request
                            is_ping = true;
                            DBG("We have received a ping reply via LoRa from address " + String(sourceMAC, HEX));
                        }
                        else if (receiveData[0].param == 0)
                        {
                            DBG("We have received a ping request from 0x" + String(sourceMAC, HEX) + ", Replying.");
                            SystemPacket pingReply = {.cmd = cmd_ping, .param = 1};
                            transmitLoRa(&sourceMAC, &pingReply, 1);
                        }
                    }
                    else
                    { // data we have received is not yet programmed.  How we handle is future enhancement.
                        DBG("Received some LoRa SystemPacket data that is not yet handled.  To be handled in future enhancement.");
                        DBG("ln: " + String(ln) + "data type: " + String(receiveData[0].cmd));
                    }
                    ackOkLoRaMsg++;
                    return CRC_OK;
                }
                else
                {
                    DBG("ACK Received CRC Mismatch! Packet CRC is 0x" + String(packetCRC, HEX) + ", Calculated CRC is 0x" + String(calcCRC, HEX));
                    return CRC_BAD;
                }
            }
        }
        else
        {
            // DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received from address 0x" + String(sourceMAC, HEX) + " destined for node address 0x" + String(destMAC, HEX));
            //  printLoraPacket(packet,sizeof(packet));
            return CRC_NULL;
        }
    }
    else
    {
        if (packetSize != 0)
        {
            // DBG("Incoming LoRa packet of " + String(packetSize) + "bytes not processed.");
            //  uint8_t packet[packetSize];
            //  radio.readData((uint8_t *)&packet, packetSize);
            //  printLoraPacket(packet,sizeof(packet));
            return CRC_NULL;
        }
    }
#endif // USE_LORA
    return CRC_NULL;
}
void printLoraPacket(uint8_t *p, int size)
{
    printf("Printing packet of size %d.", size);
    for (int i = 0; i < size; i++)
    {
        if (i % 2 == 0)
            printf("\n%02d: ", i);
        printf("%02X ", p[i]);
    }
    printf("\n");
}
