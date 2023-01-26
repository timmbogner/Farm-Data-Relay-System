#ifdef USE_LORA
#include <RadioLib.h>

#define GLOBAL_ACK_TIMEOUT  400  // LoRa ACK timeout in ms. (Minimum = 200)
#define GLOBAL_LORA_RETRIES 2    // LoRa ACK automatic retries [0 - 3]
#define GLOBAL_LORA_TXPWR   17   // LoRa TX power in dBm (: +2dBm - +17dBm (for SX1276-7) +20dBm (for SX1278))

// select LoRa band configuration
#if defined(LORA_FREQUENCY)
#define FDRS_LORA_FREQUENCY LORA_FREQUENCY
#else
#define FDRS_LORA_FREQUENCY GLOBAL_LORA_FREQUENCY
#endif //LORA_FREQUENCY

// select LoRa SF configuration
#if defined(LORA_SF)
#define FDRS_LORA_SF LORA_SF
#else
#define FDRS_LORA_SF GLOBAL_LORA_SF
#endif //LORA_SF

// select LoRa ACK configuration
#if defined(LORA_ACK) || defined(GLOBAL_LORA_ACK)
#define FDRS_LORA_ACK
#endif //LORA_ACK

// select LoRa ACK Timeout configuration
#if defined(LORA_ACK_TIMEOUT)
#define FDRS_ACK_TIMEOUT LORA_ACK_TIMEOUT
#else
#define FDRS_ACK_TIMEOUT GLOBAL_ACK_TIMEOUT
#endif //LORA_ACK_TIMEOUT

// select LoRa Retry configuration
#if defined(LORA_RETRIES)
#define FDRS_LORA_RETRIES LORA_RETRIES
#else
#define FDRS_LORA_RETRIES GLOBAL_LORA_RETRIES
#endif //LORA_RETRIES

// select  LoRa Tx Power configuration
#if defined(LORA_TXPWR)
#define FDRS_LORA_TXPWR LORA_TXPWR
#else
#define FDRS_LORA_TXPWR GLOBAL_LORA_TXPWR
#endif //LORA_TXPWR

// select  LoRa BANDWIDTH configuration
#if defined(LORA_BANDWIDTH)
#define FDRS_LORA_BANDWIDTH LORA_BANDWIDTH
#else
#define FDRS_LORA_BANDWIDTH GLOBAL_LORA_BANDWIDTH
#endif //LORA_BANDWIDTH

// select  LoRa Coding Rate configuration
#if defined(LORA_CR)
#define FDRS_LORA_CR LORA_CR
#else
#define FDRS_LORA_CR GLOBAL_LORA_CR
#endif //LORA_CR

// select  LoRa SyncWord configuration
#if defined(LORA_SYNCWORD)
#define FDRS_LORA_SYNCWORD LORA_SYNCWORD
#else
#define FDRS_LORA_SYNCWORD GLOBAL_LORA_SYNCWORD
#endif //LORA_SYNCWORD

const uint8_t lora_size   = 256 / sizeof(DataReading);

RADIOLIB_MODULE radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);
bool transmitFlag = false;// flag to indicate transmission or reception state
volatile bool enableInterrupt = true;// disable interrupt when it's not needed
volatile bool operationDone = false;// flag to indicate that a packet was sent or received

uint16_t LoRa1 =         ((mac_prefix[4] << 8) | LORA_NEIGHBOR_1);  // Use 2 bytes for LoRa addressing instead of previous 3 bytes
uint16_t LoRa2 =         ((mac_prefix[4] << 8) | LORA_NEIGHBOR_2);
uint16_t loraGwAddress = ((selfAddress[4] << 8) | selfAddress[5]); // last 2 bytes of gateway address
uint16_t loraBroadcast = 0xFFFF;
unsigned long receivedLoRaMsg = 0;  // Number of total LoRa packets destined for us and of valid size
unsigned long ackOkLoRaMsg = 0;     // Number of total LoRa packets with valid CRC
DataReading LORAGbuffer[256];
uint8_t lenLORAG = 0;
uint32_t timeLORAG = 0;
DataReading LORA1buffer[256];
uint8_t lenLORA1 = 0;
uint32_t timeLORA1 = 0;
DataReading LORA2buffer[256];
uint8_t lenLORA2 = 0;
uint32_t timeLORA2 = 0;

#endif //USE_LORA

// Function prototypes
void transmitLoRa(uint16_t*, DataReading*, uint8_t);
void transmitLoRa(uint16_t*, SystemPacket*, uint8_t);
static uint16_t crc16_update(uint16_t, uint8_t);

// CRC16 from https://github.com/4-20ma/ModbusMaster/blob/3a05ff87677a9bdd8e027d6906dc05ca15ca8ade/src/util/crc16.h#L71

/** @ingroup util_crc16
    Processor-independent CRC-16 calculation.
    Polynomial: x^16 + x^15 + x^2 + 1 (0xA001)<br>
    Initial value: 0xFFFF
    This CRC is normally used in disk-drive controllers.
    @param uint16_t crc (0x0000..0xFFFF)
    @param uint8_t a (0x00..0xFF)
    @return calculated CRC (0x0000..0xFFFF)
*/

static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
  int i;

  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }

  return crc;
}
#ifdef USE_LORA

#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }
  // we sent or received  packet, set the flag
  operationDone = true;
}

void transmitLoRa(uint16_t* destMac, DataReading * packet, uint8_t len) {
  uint16_t calcCRC = 0x0000;

  uint8_t pkt[6 + (len * sizeof(DataReading))];
  
  pkt[0] = (*destMac >> 8);       // high byte of destination MAC
  pkt[1] = (*destMac & 0x00FF);   // low byte of destination MAC
  pkt[2] = selfAddress[4];    // high byte of source MAC (ourselves)
  pkt[3] = selfAddress[5];    // low byte of source MAC
  memcpy(&pkt[4], packet, len * sizeof(DataReading));   // copy data portion of packet
  for(int i = 0; i < (sizeof(pkt) - 2); i++) {  // Last 2 bytes are CRC so do not include them in the calculation itself
    //printf("CRC: %02X : %d\n",calcCRC, i);
    calcCRC = crc16_update(calcCRC, pkt[i]);
  }
  pkt[(len * sizeof(DataReading) + 4)] = (calcCRC >> 8); // Append calculated CRC to the last 2 bytes of the packet
  pkt[(len * sizeof(DataReading) + 5)] = (calcCRC & 0x00FF);
  DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, HEX) + " to LoRa MAC 0x" + String(*destMac, HEX));
  //printLoraPacket(pkt,sizeof(pkt));
  int state = radio.startTransmit(pkt,sizeof(pkt));
      transmitFlag = true;
  if (state == RADIOLIB_ERR_NONE) {
  } else {
    DBG(" failed, code " + String(state));
    while (true);
  }
}
void transmitLoRa(uint16_t* destMac, SystemPacket * packet, uint8_t len) {
  uint16_t calcCRC = 0x0000;

  uint8_t pkt[6 + (len * sizeof(SystemPacket))];

  pkt[0] = (*destMac >> 8);       // high byte of destination MAC
  pkt[1] = (*destMac & 0x00FF);   // low byte of destination MAC
  pkt[2] = selfAddress[4];    // high byte of source MAC (ourselves)
  pkt[3] = selfAddress[5];    // low byte of source MAC
  memcpy(&pkt[4], packet, len * sizeof(SystemPacket));   // copy data portion of packet
  for(int i = 0; i < (sizeof(pkt) - 2); i++) {  // Last 2 bytes are CRC so do not include them in the calculation itself
    //printf("CRC: %02X : %d\n",calcCRC, i);
    calcCRC = crc16_update(calcCRC, pkt[i]);
}
  calcCRC = crc16_update(calcCRC, 0xA1); // No ACK for SystemPacket messages so generate new CRC with 0xA1
  pkt[(len * sizeof(SystemPacket) + 4)] = (calcCRC >> 8); // Append calculated CRC to the last 2 bytes of the packet
  pkt[(len * sizeof(SystemPacket) + 5)] = (calcCRC & 0x00FF);
  DBG("Transmitting LoRa message of size " + String(sizeof(pkt)) + " bytes with CRC 0x" + String(calcCRC, HEX) + " to LoRa MAC 0x" + String(*destMac, HEX));
  //printLoraPacket(pkt,sizeof(pkt));
  int state = radio.startTransmit(pkt,sizeof(pkt));
      transmitFlag = true;
  if (state == RADIOLIB_ERR_NONE) {
  } else {
    DBG(" failed, code " + String(state));
    while (true);
  }
}
#endif //USE_LORA

void printLoraPacket(uint8_t* p,int size) {
  printf("Printing packet of size %d.",size);
  for(int i = 0; i < size; i++ ) {
    if(i % 2 == 0) printf("\n%02d: ", i);
    printf("%02X ", p[i]);
  }
  printf("\n");
}

void begin_lora() {
  #ifdef USE_LORA
  int state = radio.begin(FDRS_LORA_FREQUENCY, FDRS_LORA_BANDWIDTH, FDRS_LORA_SF, FDRS_LORA_CR, FDRS_LORA_SYNCWORD, FDRS_LORA_TXPWR, 8, 0);
  if (state == RADIOLIB_ERR_NONE) {
    DBG("RadioLib initialization successful!");
  } else {
    DBG("RadioLib initialization failed, code " + String(state));
    while (true);
  }
  radio.setDio0Action(setFlag);
  radio.setCRC(false);
  DBG("LoRa Initialized. Frequency: " + String(FDRS_LORA_FREQUENCY) + "  Bandwidth: " + String(FDRS_LORA_BANDWIDTH) + "  SF: " + String(FDRS_LORA_SF) + "  CR: " + String(FDRS_LORA_CR) + "  SyncWord: " + String(FDRS_LORA_SYNCWORD) + "  Tx Power: " + String(FDRS_LORA_TXPWR) + "dBm");
  state = radio.startReceive();  // start listening for LoRa packets
  if (state == RADIOLIB_ERR_NONE) {
  } else {
    DBG(" failed, code " + String(state));
    while (true);
  }
#endif // USE_LORA
}

crcResult getLoRa() {
#ifdef USE_LORA
    
  int packetSize = radio.getPacketLength();
  if((((packetSize - 6) % sizeof(DataReading) == 0) || ((packetSize - 6) % sizeof(SystemPacket) == 0)) && packetSize > 0) {  // packet size should be 6 bytes plus multiple of size of DataReading
    uint8_t packet[packetSize];
    uint16_t packetCRC = 0x0000; // CRC Extracted from received LoRa packet
    uint16_t calcCRC = 0x0000; // CRC calculated from received LoRa packet
    uint16_t sourceMAC = 0x0000;
    uint16_t destMAC = 0x0000;
  
    radio.readData((uint8_t *)&packet, packetSize);

    destMAC = (packet[0] << 8) | packet[1];
    sourceMAC = (packet[2] << 8) | packet[3];
    packetCRC = ((packet[packetSize - 2] << 8) | packet[packetSize - 1]);
    //DBG("Packet Address: 0x" + String(packet[0], HEX) + String(packet[1], HEX) + " Self Address: 0x" + String(selfAddress[4], HEX) + String(selfAddress[5], HEX));
    if (destMAC == (selfAddress[4] << 8 | selfAddress[5])) {   //Check if addressed to this device (2 bytes, bytes 1 and 2)
      //printLoraPacket(packet,sizeof(packet));
      if(receivedLoRaMsg != 0){  // Avoid divide by 0
        DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(radio.getRSSI()) + "dBm, SNR: " + String(radio.getSNR()) + "dB, PacketCRC: 0x" + String(packetCRC, HEX) + ", Total LoRa received: " + String(receivedLoRaMsg) + ", CRC Ok Pct " + String((float)ackOkLoRaMsg/receivedLoRaMsg*100) + "%");
      }
      else {
        DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(radio.getRSSI()) + "dBm, SNR: " + String(radio.getSNR()) + "dB, PacketCRC: 0x" + String(packetCRC, HEX) + ", Total LoRa received: " + String(receivedLoRaMsg));
      }
      receivedLoRaMsg++;
      // Evaluate CRC
      for(int i = 0; i < (packetSize - 2); i++) { // Last 2 bytes of packet are the CRC so do not include them in calculation
        //printf("CRC: %02X : %d\n",calcCRC, i);
        calcCRC = crc16_update(calcCRC, packet[i]);
      }
      if((packetSize - 6) % sizeof(DataReading) == 0) { // DataReading type packet
        if(calcCRC == packetCRC) {
          SystemPacket ACK = { .cmd = cmd_ack, .param = CRC_OK };
          DBG("CRC Match, sending ACK packet to sensor 0x" + String(sourceMAC, HEX) + "(hex)");
          transmitLoRa(&sourceMAC, &ACK, 1);  // Send ACK back to source
        }
        else if(packetCRC == crc16_update(calcCRC,0xA1)) { // Sender does not want ACK and CRC is valid
          DBG("Sensor address 0x" + String(sourceMAC,16) + "(hex) does not want ACK");
        }
        else {
          SystemPacket NAK = { .cmd = cmd_ack, .param = CRC_BAD };
          // Send NAK packet to sensor
          DBG("CRC Mismatch! Packet CRC is 0x" + String(packetCRC, HEX) + ", Calculated CRC is 0x" + String(calcCRC, HEX) + " Sending NAK packet to sensor 0x" + String(sourceMAC, HEX) + "(hex)");
          transmitLoRa(&sourceMAC, &NAK, 1); // CRC did not match so send NAK to source
          newData = event_clear;  // do not process data as data may be corrupt
          return CRC_BAD;  // Exit function and do not update newData to send invalid data further on
        }
          memcpy(&theData, &packet[4], packetSize - 6);   //Split off data portion of packet (N - 6 bytes (6 bytes for headers and CRC))
          ln = (packetSize - 6) / sizeof(DataReading);
          ackOkLoRaMsg++;
        if (memcmp(&sourceMAC, &LoRa1, 2) == 0) {      //Check if it is from a registered sender
          newData = event_lora1;
          return CRC_OK;
        }
        if (memcmp(&sourceMAC, &LoRa2, 2) == 0) {
          newData = event_lora2;
          return CRC_OK;
        }
        newData = event_lorag;
        return CRC_OK;
      }
      else if((packetSize - 6) % sizeof(SystemPacket) == 0) {
        uint ln = (packetSize - 6) / sizeof(SystemPacket);
        SystemPacket receiveData[ln];
    
        if(calcCRC == packetCRC) {
          memcpy(receiveData, &packet[4], packetSize - 6);   //Split off data portion of packet (N bytes)
          if(ln == 1 && receiveData[0].cmd == cmd_ack) {
            DBG("ACK Received - CRC Match");
          }
          else if(ln == 1 && receiveData[0].cmd == cmd_ping) { // We have received a ping request or reply??
            if(receiveData[0].param == 1) {  // This is a reply to our ping request
              is_ping = true;
              DBG("We have received a ping reply via LoRa from address " + String(sourceMAC, HEX));
            }
            else if(receiveData[0].param == 0) {
              DBG("We have received a ping request from 0x" + String(sourceMAC, HEX) + ", Replying.");
              SystemPacket pingReply = { .cmd = cmd_ping, .param = 1 };
              transmitLoRa(&sourceMAC, &pingReply, 1);
            }
          }
          else { // data we have received is not yet programmed.  How we handle is future enhancement.
            DBG("Received some LoRa SystemPacket data that is not yet handled.  To be handled in future enhancement.");
            DBG("ln: " + String(ln) + "data type: " + String(receiveData[0].cmd));
          }
          ackOkLoRaMsg++;
          return CRC_OK;
        }
        else if(packetCRC == crc16_update(calcCRC,0xA1)) { // Sender does not want ACK and CRC is valid
          memcpy(receiveData, &packet[4], packetSize - 6);   //Split off data portion of packet (N bytes)
          if(ln == 1 && receiveData[0].cmd == cmd_ack) {
            DBG("ACK Received - CRC Match");
          }
          else if(ln == 1 && receiveData[0].cmd == cmd_ping) { // We have received a ping request or reply??
            if(receiveData[0].param == 1) {  // This is a reply to our ping request
              is_ping = true;
              DBG("We have received a ping reply via LoRa from address " + String(sourceMAC, HEX));
            }
            else if(receiveData[0].param == 0) {
              DBG("We have received a ping request from 0x" + String(sourceMAC, HEX) + ", Replying.");
              SystemPacket pingReply = { .cmd = cmd_ping, .param = 1 };
              transmitLoRa(&sourceMAC, &pingReply, 1);
            }
          }
          else { // data we have received is not yet programmed.  How we handle is future enhancement.
            DBG("Received some LoRa SystemPacket data that is not yet handled.  To be handled in future enhancement.");
            DBG("ln: " + String(ln) + "data type: " + String(receiveData[0].cmd));
          }
          ackOkLoRaMsg++;
          return CRC_OK;
        }
        else {
          DBG("ACK Received CRC Mismatch! Packet CRC is 0x" + String(packetCRC, HEX) + ", Calculated CRC is 0x" + String(calcCRC, HEX));
          return CRC_BAD;
        }
      }
    }
    else {
      DBG("Incoming LoRa packet of " + String(packetSize) + " bytes received from address 0x" + String(sourceMAC, HEX) + " destined for node address 0x" + String(destMAC, HEX));
        // printLoraPacket(packet,sizeof(packet));
        return CRC_NULL;

    }
  }
  else {
    if(packetSize != 0) {
      DBG("Incoming LoRa packet of " + String(packetSize) + "bytes not processed.");
      // uint8_t packet[packetSize];
      // radio.readData((uint8_t *)&packet, packetSize);
      // printLoraPacket(packet,sizeof(packet));
      return CRC_NULL;

    }
  }
#endif //USE_LORA
  return CRC_NULL;
}


void bufferLoRa(uint8_t interface) {
#ifdef USE_LORA
  DBG("Buffering LoRa.");
  switch (interface) {
    case 0:
      for (int i = 0; i < ln; i++) {
        LORAGbuffer[lenLORAG + i] = theData[i];
      }
      lenLORAG += ln;
      break;
    case 1:
      for (int i = 0; i < ln; i++) {
        LORA1buffer[lenLORA1 + i] = theData[i];
      }
      lenLORA1 += ln;
      break;
    case 2:
      for (int i = 0; i < ln; i++) {
        LORA2buffer[lenLORA2 + i] = theData[i];
      }
      lenLORA2 += ln;
      break;
  }
#endif //USE_LORA
}

void releaseLoRa(uint8_t interface) {
#ifdef USE_LORA
  DBG("Releasing LoRa.");

  switch (interface) {
    case 0:
      {
        DataReading thePacket[lora_size];
        int j = 0;
        for (int i = 0; i < lenLORAG; i++) {
          if ( j > lora_size) {
            j = 0;
            transmitLoRa(&loraBroadcast, thePacket, j);
          }
          thePacket[j] = LORAGbuffer[i];
          j++;
        }
        transmitLoRa(&loraBroadcast, thePacket, j);
        lenLORAG = 0;

        break;
      }
    case 1:
      {
        DataReading thePacket[lora_size];
        int j = 0;
        for (int i = 0; i < lenLORA1; i++) {
          if ( j > lora_size) {
            j = 0;
            transmitLoRa(&LoRa1, thePacket, j);
          }
          thePacket[j] = LORA1buffer[i];
          j++;
        }
        transmitLoRa(&LoRa1, thePacket, j);
        lenLORA1 = 0;
        break;
      }
    case 2:
      {
        DataReading thePacket[lora_size];
        int j = 0;
        for (int i = 0; i < lenLORA2; i++) {
          if ( j > lora_size) {
            j = 0;
            transmitLoRa(&LoRa2, thePacket, j);
          }
          thePacket[j] = LORA2buffer[i];
          j++;
        }
        transmitLoRa(&LoRa2, thePacket, j);
        lenLORA2 = 0;

        break;
      }
  }
#endif
}

void handleLoRa(){
  #ifdef USE_LORA
  if(operationDone) { // the interrupt was triggered
    enableInterrupt = false;
    operationDone = false;
    if(transmitFlag) {  // the previous operation was transmission
      radio.startReceive();   // return to listen mode 
      enableInterrupt = true;
      transmitFlag = false;
    } else {  // the previous operation was reception
      returnCRC = getLoRa();
      if (!transmitFlag) radio.startReceive(); //return to listen if no transmission was begun.
      enableInterrupt = true;
      }
    } 
    #endif //USE_LORA 
  }

