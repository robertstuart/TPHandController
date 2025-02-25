#define XBEE_3P_B 0xCF
const int DATA_FRAME_MAX = 72;
byte rcvDataFrame[DATA_FRAME_MAX + 1];
int rcvDataFramePtr = 0;
int rcvPacketCount = 0;
int rcvDataFrameLength = 0;

/*********************************************************
 *
 * readXBee()
 *
 *     Read bytes from the XBee radio, and call
 *     interpretDataFrame() whenever there is a complete 
 *     data packet.
 *
 *********************************************************/
void readXBee() {
  static boolean escState = false;
  
  while (XBEE_SER.available() > 0) {
    byte b = XBEE_SER.read();
    if (b == 0x7e) {
      rcvPacketCount = 0;
      rcvDataFramePtr = 0;
      escState = false;
    } else {
      if (escState) {
        b = b ^ 0x20;
        escState = false;
      }
      else if (b == 0x7D) {
        escState = true;
        return;
      }

      if (rcvPacketCount == 1) rcvDataFrameLength = b * 256;
      else if (rcvPacketCount == 2) rcvDataFrameLength += b;
      else {
        if (rcvDataFramePtr < rcvDataFrameLength) {
          rcvDataFrame[rcvDataFramePtr++] = b;
         } else if (rcvDataFramePtr == rcvDataFrameLength) { // Checksum
          interpretRcvDataFrame();
          rcvDataFramePtr++;  // just in case...
        }
      }
    }
    rcvPacketCount++;
  }
}  // end readXBee()



/**************************************************************************.
 * interpretRcvDataFrame()
 **************************************************************************/
void interpretRcvDataFrame() {
  boolean isTwoPotatoe;

  switch (rcvDataFrame[0]) { // cmdID
    case 0x8A:           // Modem Status
      break;
    case 0x88:           // AT Command Response
      break;
    case 0x97:           // Remote Command Response
      break;
    case 0x8B:           // Transmit Status
      break;
    case 0x90:           // Receive Packet (A0=0)
//      Serial.println(rcvDataFrame[8],HEX);
      isTwoPotatoe = (rcvDataFrame[8] == XBEE_3P_B) ? false : true;
      doRFData(isTwoPotatoe);
      break;
    case 0x91:           // Receive Packet (A)=1)
      break;
    default:
     break;
  }
}

void doRFData(boolean isTwoPotatoe) {
//Serial.print(rcvDataFrame[1], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[2], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[3], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[4], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[5], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[6], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[7], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[8], HEX); Serial.print(" ");
//Serial.print("  ");
//Serial.print(rcvDataFrame[9], HEX); Serial.print(" ");
//Serial.print(rcvDataFrame[10], HEX);
//Serial.println();
//
  static int cmd;
  int rfPtr = 12;
  char msgVal[100];
  int msgValPtr = 0;
  if (isTwoPotatoe) msg2Time = timeMillis;
  else msg6Time = timeMillis;
  
  while (rfPtr < rcvDataFrameLength) {
    byte b = rcvDataFrame[rfPtr];
    if (b < 128) {
      msgVal[msgValPtr++] = b;
    }
    if ((b > 127) || (rfPtr == (rcvDataFrameLength - 1))) {
      if (msgValPtr > 0) {
        doMsg(cmd, msgVal, msgValPtr, isTwoPotatoe);
      }
      msgValPtr = 0;
      cmd = b;
    }
    rfPtr++;
  }
//Serial.println();
}


/**************************************************************************.
 * doMsg()
 **************************************************************************/
void doMsg(int cmd, char msgStr[], int count, boolean isTwoPotatoe) {
  int intVal;
  float floatVal;

  msgStr[count] = 0;

  switch (cmd) {
    case SEND_FPS:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2Fps = floatVal;
        else  p6Fps = floatVal;
      }
      break;
    case SEND_ROUTE_STEP:
      if (sscanf(msgStr, "%d", &intVal) > 0) {
        if (isTwoPotatoe) p2Step = intVal;
        else p2Step = intVal;
      }
      break;
    case SEND_MODE:
//      if (sscanf(msgStr, "%d", &intVal) > 0) tpMode = intVal;
      break;
    case SEND_BATT:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2BattV = floatVal;
        else p6BattV = floatVal;
      }
      break;
    case SEND_ROUTE_NAME:
    case SEND_MESSAGE:
      if (isTwoPotatoe) p2Message = String(msgStr);
      else p6Message = String(msgStr);
      break;
    case SEND_STATE:
      if (sscanf(msgStr, "%d", &intVal) > 0) {
        if (isTwoPotatoe) interpret2State(intVal);
        else interpret6State(intVal);
      }
      break;
    case SEND_V1:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2V1 = floatVal;
        else p6V1 = floatVal;
      }
      break;
    case SEND_V2:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2V2 = floatVal;
        else p6V2 = floatVal;
      }
      break;
    default:
      Serial.print("Illegal message received: "); Serial.println(cmd);
      break;

  }
}
