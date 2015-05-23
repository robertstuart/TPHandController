const int B_BUFFER_SIZE = 100;

char msgStrX[B_BUFFER_SIZE];
int msgStrPtrX = 0;
int msgCmdX = 0;
boolean isMessageInProgressX = false;


void readXBee() {
  while (Serial1.available()) {
    byte b = Serial1.read();
    if (b >= 128) {
      msgStrPtrX = 0;
      msgCmdX = b;
      isMessageInProgressX = true;
    }
    else {
      if (isMessageInProgressX) {
        if (msgStrPtrX >= B_BUFFER_SIZE) {
          isMessageInProgressX = false;
        } else if (b == 0) {
          doMsg(msgCmdX, msgStrX, msgStrPtrX);
        } 
        else {
          msgStrX[msgStrPtrX++] = b;
        }
      }
    }
  }
}

// returns true if a state message is received
void doMsg(int cmd, char msgStr[], int count) {
  int intVal;
  float floatVal;
  boolean booleanVal;
  
  msgStr[count] = 0;
  msgTime = timeMilliseconds;
  switch (cmd) {
    case SEND_FPS:
      if (sscanf(msgStr, "%f", &floatVal) > 0) tpFps = floatVal;
      break;
    case SEND_PITCH:
      if (sscanf(msgStr, "%f", &floatVal) > 0) tpPitch = floatVal;
      break;
    case SEND_HEADING:
      if (sscanf(msgStr, "%f", &floatVal) > 0) tpHeading = floatVal;
      break;
    case SEND_SONAR:
      if (sscanf(msgStr, "%f", &floatVal) > 0) tpSonarDistance = floatVal;
      break;
    case SEND_ROUTE_STEP:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpRouteStep = intVal;
      break;
    case SEND_MODE:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpMode = intVal;
      break;
    case SEND_BATT:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpBatt = intVal;
      break;
   case SEND_VALSET:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpValSet = intVal;
      break;
   case SEND_MESSAGE:
      message = String(msgStr);
      break;
    case SEND_STATE:
      if (sscanf(msgStr, "%d", &intVal) > 0) tpState = intVal;
      updateAll(); // This should be the last message in a series.
      break;
    default:
      Serial.println("Illegal message received: " + cmd);
      break;

  }
}

