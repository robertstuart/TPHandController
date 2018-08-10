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
  if (isTwoPotatoe) msg2Time = timeMilliseconds;
  else msg6Time = timeMilliseconds;
  
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
  boolean booleanVal;
  msgStr[count] = 0;
//Serial.print(cmd); 
//Serial.print("\t");
//Serial.println(msgStr);
  
  switch (cmd) {
    case SEND_FPS:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2Fps = floatVal;
        else  p6Fps = floatVal;
      }
      break;
    case SEND_SONAR_L:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2SonarDistanceL = floatVal;
        else p3SonarDistanceL = floatVal;
      }
      break;
    case SEND_SONAR_F:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2SonarDistanceF = floatVal;
        else p3SonarDistanceF = floatVal;
      }
      break;
    case SEND_SONAR_R:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2SonarDistanceR = floatVal;
        else p3SonarDistanceR = floatVal;
      }
      break;
    case SEND_ROUTE_STEP:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpRouteStep = intVal;
      break;
    case SEND_MODE:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpMode = intVal;
      break;
    case SEND_BATT:
      if (sscanf(msgStr, "%f", &floatVal) > 0) {
        if (isTwoPotatoe) p2Batt = floatVal;
        else p6Batt = floatVal;
      }
      break;
    case SEND_ROUTE_NAME:
    case SEND_MESSAGE:
      if (isTwoPotatoe) p2Message = String(msgStr);
      else p3Message = String(msgStr);
      break;
    case SEND_STATE:
      if (sscanf(msgStr, "%d", &intVal) > 0) {
        if (isTwoPotatoe) interpret2State(intVal);
        else interpret6State(intVal);
      }
      break;
    case SEND_X:
//      if (sscanf(msgStr, "%f", &floatVal) > 0) tpX = String(msgStr);
      break;
    case SEND_Y:
//      if (sscanf(msgStr, "%f", &floatVal) > 0) tpY = String(msgStr);
      break;
    case SEND_V1:
      if (sscanf(msgStr, "%f", &floatVal) > 0) v1 = floatVal;
      break;
    case SEND_V2:
      if (sscanf(msgStr, "%f", &floatVal) > 0) v2 = floatVal;
      break;
    default:
      Serial.print("Illegal message received: "); Serial.println(cmd);
      break;

  }
}

