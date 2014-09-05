
const int PACKET_DELIM = 0;
const int PACKET_MSB = 1;
const int PACKET_LSB = 2;
const int PACKET_API_ID = 3;
const int PACKET_RX = 4;
const int PACKET_MS = 5;
const int PACKET_TXS = 6;
int packetInProgress = PACKET_DELIM;
const int PACKETBYTE_MAX = 105;
byte packetByteArray[PACKETBYTE_MAX];
int packetLength = 0;
int dataLength = 0;
int packetByteCount = 0;
int packetSource;
int signalStrength;
int dataPtr = 0;



/*********************************************************************
 * 
 * readXbee()  
 *
 *   reads byte(s) from UART.  When the start of a new packet from 
 *   a controller is received, the checksum is flushed (starting a
 *   new transmit) and true is returned indicating that the UART
 *   will be quiet before it starts receiving the next packet.
 * 
 ********************************************************************/
boolean readXBee() {
  static boolean escState = false;
  
  while (Serial.available() > 0) {
    byte b = Serial.read();
    
    // Fix escape sequences
    if (packetInProgress != PACKET_DELIM) {
      if (escState) {
        b = b ^ 0x20;
        escState = false;
      }
      else if (b == 0x7D) {
        escState = true;
        return false;
      }
    }
    
    switch(packetInProgress) {
    case PACKET_DELIM:
      if (b == 0x7E) {
        packetByteCount = 0;
        packetInProgress = PACKET_MSB;
//        flushChecksum();
        sendResponse();  // do reply
      }
      break;
    case PACKET_MSB:
      packetLength = b * 256;
      packetInProgress = PACKET_LSB;
      break;
    case PACKET_LSB:
      packetLength += b;
      packetInProgress = PACKET_API_ID;
      break;
    case PACKET_API_ID:
      switch (b) {
      case 0x81:
        packetInProgress = PACKET_RX;
        break;
      case 0x89:
        packetInProgress = PACKET_TXS;
        break;
      case 0x8A:
        packetInProgress = PACKET_MS;
        break;
      default:
        packetInProgress = PACKET_DELIM;
      }
      break;
    case PACKET_RX:
      packetInProgress = doRx(b);
      if (packetInProgress == PACKET_DELIM) {
        return true;  // We have just received a complete packet.
      }
      break;
    case PACKET_TXS:
      packetInProgress = doTxs(b);
      break;
    case PACKET_MS:
      packetInProgress = doMs(b);
      break;
    } // end switch(packetInProgress)
  } // end while(dataReady)
  return false;
} // end readXBee()


int doMs(int b) {
  if (packetByteCount++ < 2) {
    return PACKET_MS;
  }
  return PACKET_DELIM;
}

int doTxs(int b) {
  switch (packetByteCount++) {
  case 0:
    return PACKET_TXS;
  case 1:
    return PACKET_TXS;
  }
  return PACKET_DELIM;
}

// Data packet in progress
int doRx(int b) {
  switch (packetByteCount++) {
  case 0:
    packetSource = b * 256;
    break;
  case 1:
    packetSource += b;
    break;
  case 2:
    signalStrength = b;
    break;
  case 3: // Options
    dataLength = packetLength - 5;  // Subtract out non-data bytes.
    dataPtr = 0;
    break;
    // Data or checksum after this point.
  default:
    if (dataPtr == dataLength)	{ // Checksum?
      newPacket();
      return PACKET_DELIM;
    } 
    else if (dataPtr <= 100) {
      packetByteArray[dataPtr++] = (byte) b;
    }
  } 
  return PACKET_RX;
} // end doRX()

/*********************************************************************
 * 
 * newPacket()  ----New packet received from Serial.
 * 
 ********************************************************************/
void newPacket() {
  tpMode = packetByteArray[TP_SEND_MODE_STATUS];
  if (tpMode == BLOCK_DATA) return;
  
  // Note: lastOffset is set to the offset AFTER the last value.
  msgTime = timeMilliseconds;
  tpBattVolt = get2Byte(packetByteArray, TP_SEND_BATTERY);
  tpValSet = packetByteArray[TP_SEND_VALSET_STATUS];
  tpState = packetByteArray[TP_SEND_STATE_STATUS];
}

int get2Byte(byte array[], int index) {
  int b1 = array[index];
  int b2 = array[index + 1] & 0xFF;
  return (b1 << 8) + b2;
}	
//int get4Byte(byte array[], int index) {
//  long b1 = array[index];
//  long b2 = array[index + 1] & 0xFF;
//  long b3 = array[index + 2] & 0xFF;
//  long b4 = array[index + 3] & 0xFF;
//  return (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
//}


