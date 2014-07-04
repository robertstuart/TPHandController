
byte transmitBuffer[120] = {
  0x7E, 0, 0}; // Start delimiter, count MSB, count LSB
byte tXframeDataHeader[] = {
  0x01, 0, 0, 0, 0x01}; // API id, Frame id, dest MSB, dest LSB, Options
byte sendArray[100];
int transmitBufferLength = 1;  // So the flushChecksum() doesn't send 7E
int transmitBufferPtr = 0;
unsigned long transmitNextWriteTime = 0UL;
int tpMsgRcvType = 0;
int tpMsgRcvVal = 0;

/*********************************************************
 *
 * sendMsg()
 *
 *      Sends a message to TP.  If there has been to ack to the
 *      previous message, it goes ahead and sends it anyway.
 *
 *********************************************************/
void sendMsg(int rType, int rVal) {
  tpMsgRcvType = rType;
  tpMsgRcvVal = rVal;  
}

///*********************************************************
// *
// * ackMsg()
// *
// *      An acknowledge received from TP.  At the moment, this does
// *      nothing more than set the send value to zero.  Later, we could
// *      have a queue.
// *
// *********************************************************/
//void ackMsg(int tpMsgRcvType, int tpMsgRcvVal) {
//  if ((tpMsgRcvType == tpMsgRcvType) && (tpMsgRcvVal == tpMsgRcvVal)) {
//    tpMsgRcvType = TP_RCV_MSG_NULL;
//    tpMsgRcvVal = 0;
//  }
//}


/*********************************************************
 *
 * sendResponse()
 *
 *     Called when packet received.
 *
 *********************************************************/
void sendResponse() {
  set1Byte(sendArray, TP_RCV_X, x);
  set1Byte(sendArray, TP_RCV_Y, y);
  sendArray[TP_RCV_MSG_TYPE] = tpMsgRcvType;
  set2Byte(sendArray, TP_RCV_MSG_VAL, tpMsgRcvVal);
  sendTXFrame(XBEE_TWOPOTATOE, sendArray, TP_RCV_MAX);
}



/*********************************************************
 *
 * sendTXFrame()
 *
 *     Send a TX frame to the specified destination.
 *     The data wlll be int the rfData.  Prepend
 *     the API id, Frame Id, Destination Address, and 
 *     Options bytes
 *     rfDataLenth is always the offset AFTER the last
 *     array value.
 *
 *********************************************************/
void sendTXFrame(int dest, byte rfData[], int rfDataLength) {  
  tXframeDataHeader[2] = dest/256;    // MSB
  tXframeDataHeader[3] = dest & 0xFF; // LSB
  sendFrame(tXframeDataHeader, 5, rfData, rfDataLength);
}


/*********************************************************
 *
 * sendFrame()
 *
 *     Send the API frame with the given Data Header
 *     and Data. Prepend the Start Delimiter (0cFE)
 *     and the Length bytes.  Append the Checksum
 *     before sending.
 *
 *********************************************************/
void sendFrame(byte cmdDataHeader[], int cmdDataHeaderLength, byte cmdData[], int cmdDataLength) {
  unsigned int sum = 0;

  // Set the length in bytes 2 & 3.
  transmitBuffer[1] = 0;
  transmitBuffer[2] = cmdDataHeaderLength + cmdDataLength;

  // Compute the checksum.
  for (int i = 0; i < cmdDataHeaderLength; i++) {
    sum += cmdDataHeader[i];
  }
  for (int i = 0; i < cmdDataLength; i++) {
    sum += cmdData[i];
  }
  byte checkSum = 0xFF - sum;
  // Copy to a single buffer and start the first byte of the transmit.
  for (int i = 0; i < cmdDataHeaderLength; i++) {
    transmitBuffer[i + 3] = cmdDataHeader[i];
  }
  for (int i = 0; i < cmdDataLength; i++) {
    transmitBuffer[i + 3 + cmdDataHeaderLength] = cmdData[i];
  }

  transmitBufferLength = 3 + cmdDataHeaderLength + cmdDataLength;
  transmitBuffer[transmitBufferLength] = checkSum;
  Serial.write(transmitBuffer, transmitBufferLength + 1);
//flushChecksum();
}

//void flushChecksum() {
//  Serial.write(transmitBuffer[transmitBufferLength]);
//}

void set1Byte(byte array[], int offset, int value) {
  value += 127;
  if (value < 0) {
    value = 0;
  } 
  else if (value > 255) {
    value = 255;
  }
  array[offset] = (byte) value;		
}
void set2Byte(byte array[], int offset, int value) {
  long v = ((long) value) + 32767;
  if (v < 0) {
    v = 0;
  } 
  else if (v > 65535) {
    v = 65535;
  }
  array[offset + 1] = (byte) (v & 0xFFL);
  v = v >> 8;
  array[offset] = (byte) v;
}




