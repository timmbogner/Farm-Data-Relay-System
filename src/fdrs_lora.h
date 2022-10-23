
#ifdef USE_LORA
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
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
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
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&pkt, sizeof(pkt));
  LoRa.endPacket();
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
  DBG("Initializing LoRa!");
#ifdef ESP32
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
#endif
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(FDRS_BAND)) {
    DBG(" Initialization failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(FDRS_SF);
  LoRa.setTxPower(FDRS_TXPWR);
  DBG("LoRa Initialized. Band: " + String(FDRS_BAND) + " SF: " + String(FDRS_SF) + " Tx Power: " + String(LORA_TXPWR) + " dBm");

#endif // USE_LORA
}

crcResult getLoRa() {
#ifdef USE_LORA
  int packetSize = LoRa.parsePacket();
  if((((packetSize - 6) % sizeof(DataReading) == 0) || ((packetSize - 6) % sizeof(SystemPacket) == 0)) && packetSize > 0) {  // packet size should be 6 bytes plus multiple of size of DataReading
    uint8_t packet[packetSize];
    uint16_t packetCRC = 0x0000; // CRC Extracted from received LoRa packet
    uint16_t calcCRC = 0x0000; // CRC calculated from received LoRa packet
    uint16_t sourceMAC = 0x0000;
    uint16_t destMAC = 0x0000;
  
    LoRa.readBytes((uint8_t *)&packet, packetSize);
    
    destMAC = (packet[0] << 8) | packet[1];
    sourceMAC = (packet[2] << 8) | packet[3];
    packetCRC = ((packet[packetSize - 2] << 8) | packet[packetSize - 1]);
    //DBG("Packet Address: 0x" + String(packet[0], HEX) + String(packet[1], HEX) + " Self Address: 0x" + String(selfAddress[4], HEX) + String(selfAddress[5], HEX));
    if (destMAC == (selfAddress[4] << 8 | selfAddress[5])) {   //Check if addressed to this device (2 bytes, bytes 1 and 2)
      //printLoraPacket(packet,sizeof(packet));
      if(receivedLoRaMsg != 0){  // Avoid divide by 0
        DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(LoRa.packetRssi()) + "dBm, SNR: " + String(LoRa.packetSnr()) + "dB, PacketCRC: 0x" + String(packetCRC, HEX) + ", Total LoRa received: " + String(receivedLoRaMsg) + ", CRC Ok Pct " + String((float)ackOkLoRaMsg/receivedLoRaMsg*100) + "%");
      }
      else {
        DBG("Incoming LoRa. Size: " + String(packetSize) + " Bytes, RSSI: " + String(LoRa.packetRssi()) + "dBm, SNR: " + String(LoRa.packetSnr()) + "dB, PacketCRC: 0x" + String(packetCRC, HEX) + ", Total LoRa received: " + String(receivedLoRaMsg));
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
    }
  }
  else {
    if(packetSize != 0) {
      DBG("Incoming LoRa packet of " + String(packetSize) + "bytes not processed.");
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

void releaseSerial() {
  DBG("Releasing Serial.");
  DynamicJsonDocument doc(24576);
  for (int i = 0; i < lenSERIAL; i++) {
    doc[i]["id"]   = SERIALbuffer[i].id;
    doc[i]["type"] = SERIALbuffer[i].t;
    doc[i]["data"] = SERIALbuffer[i].d;
  }
  serializeJson(doc, UART_IF);
  UART_IF.println();
  lenSERIAL = 0;
}

