const int RF_DATA_SIZE = 72;
byte rfData2[RF_DATA_SIZE];
byte rfData6[RF_DATA_SIZE];
int rfData2Ptr = 0;
int rfData6Ptr = 0;

int dumpPtr, dumpEnd;
int tickDumpPtr, tickDumpEnd;

void send2Potatoe() {
  queue2Msg(RCV_JOYX, 2, joyX);
  queue2Msg(RCV_JOYY, 2, joyY);
  xTransmitRequest(rfData2, rfData2Ptr, true);
  rfData2Ptr = 0;
}


void send6Potatoe() {
  queue6Msg(RCV_JOYX_I, (int) (joyX * 100.0));
  queue6Msg(RCV_JOYY_I, (int) (joyY * 100.0));
  xTransmitRequest(rfData6, rfData6Ptr, false);
  rfData6Ptr = 0;
}



void queue2Msg(int cmd, int precision, double val) {
  char buf[20];
  int len = sprintf(buf, "%.*f", precision, val);
  add2Message(cmd, buf, len);
}
void queue6Msg(int cmd, int precision, double val) {
  char buf[20];
  int len = sprintf(buf, "%.*f", precision, val);
  add6Message(cmd, buf, len);
}



void queue2Msg(int cmd, int val) {
  char buf[10];
  int len = sprintf(buf, "%d", val);
  add2Message(cmd, buf, len);
}
void queue6Msg(int cmd, int val) {
  char buf[10];
  int len = sprintf(buf, "%d", val);
  add6Message(cmd, buf, len);
}



void queue2Msg(int cmd, String val) {
  char buf[50];
  int len = val.length();
  if (len >= 50) return;
  val.toCharArray(buf, len);
  add2Message(cmd, buf, len);
}
void queue6Msg(int cmd, String val) {
  char buf[50];
  int len = val.length();
  if (len >= 50) return;
  val.toCharArray(buf, len);
  add6Message(cmd, buf, len);
}


/**************************************************************************.
 * add?Message()
 *                Add message to rfData
 **************************************************************************/
void add2Message(int cmd, char buf[], int len) {
  if ((len + 1 + rfData2Ptr) >= RF_DATA_SIZE) return;
  rfData2[rfData2Ptr++] = cmd;
  for (int i = 0; i < len; i++) {
    rfData2[rfData2Ptr++] = buf[i];
  }
}
void add6Message(int cmd, char buf[], int len) {
  if ((len + 1 + rfData6Ptr) >= RF_DATA_SIZE) return;
  rfData6[rfData6Ptr++] = cmd;
  for (int i = 0; i < len; i++) {
    rfData6[rfData6Ptr++] = buf[i];
  }
}


/**************************************************************************.
 * xTransmitRequest()
 *                    Create a Transmit Request data frame from the 
 *                    rfDataFrame and send it out.
 **************************************************************************/
void xTransmitRequest(byte rfFrame[], int rfLength, boolean is2Potatoe) { 
  static byte txRequestDataFrame[100];
  static int frameId = 0;
  unsigned int sh, sl;
  frameId++;
  frameId = frameId % 200;   // ID cycles 1-200
  if (is2Potatoe) {
    sh = XBEE_2P_SH;
    sl = XBEE_2P_SL;
  } else {
    sh = XBEE_6P_SH;
    sl = XBEE_6P_SL;
  }
  txRequestDataFrame[0] = 0x10;  // API identifier value
  txRequestDataFrame[1] = frameId + 1;
  txRequestDataFrame[2] = (sh >> 24) & 0x000000FF;
  txRequestDataFrame[3] = (sh >> 16) & 0x000000FF;
  txRequestDataFrame[4] = (sh >> 8) & 0x000000FF;
  txRequestDataFrame[5] = sh & 0x000000FF;
  txRequestDataFrame[6] = (sl >> 24) & 0x000000FF;
  txRequestDataFrame[7] = (sl >> 16) & 0x000000FF;
  txRequestDataFrame[8] = (sl >> 8) & 0x000000FF;
  txRequestDataFrame[9] = sl & 0x000000FF;
  txRequestDataFrame[10] = 0x24;  // 16-bit address unknown
  txRequestDataFrame[11] = 0x56;  // 16-bit address unknown
  txRequestDataFrame[12] = 0;     // Raduis
  txRequestDataFrame[13] = 0;     // 0ptions

  for (int i = 0; i < rfLength; i++) {
    txRequestDataFrame[i + 14] = rfFrame[i];
  }
  xTransmitUartFrame(txRequestDataFrame, rfLength + 14);  
}



/**************************************************************************.
 *
 * xTransmitUartFrame()
 *
 *     Set all frame bytes and send out with appropriate
 *     characters escaped.
 **************************************************************************/
void xTransmitUartFrame(byte dataFrame[], int dataFrameLength) {
  static byte preEscPack[100];
  static byte uartXmitFrame[200];
  int sum = 0;

  // Compute the checksum.
  for (int i = 0; i < dataFrameLength; i++) {
    sum += dataFrame[i];
  }  
  byte checkSum = 0xFF - (sum & 0xFF);

  // Fill out preEscPack with the array that must be escaped. That is, minus FE.
  preEscPack[0] = 0;
  preEscPack[1] = dataFrameLength;
  for (int i = 0; i < dataFrameLength; i++) {
    preEscPack[i + 2] = dataFrame[i];
  }
  preEscPack[dataFrameLength + 2] = checkSum;

  // Now put it into a single array with the escaped characters.
  int oPtr = 1;
  uartXmitFrame[0] = 0x7E;
  for (int i = 0; i < (dataFrameLength + 3); i++) {
    int b = preEscPack[i];
    if ((b == 0x7E) || (b == 0x7D) || (b == 0x11) || (b == 0x13)) {
      uartXmitFrame[oPtr++] = 0x7D;
      uartXmitFrame[oPtr++] = b ^ 0x20;
    }
    else {
      uartXmitFrame[oPtr++] = b;
    }
  }
  XBEE_SER.write(uartXmitFrame, oPtr);
}
