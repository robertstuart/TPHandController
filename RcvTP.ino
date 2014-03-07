const int MAX_PACKET_SIZE = 100;

byte packetBytes[MAX_PACKET_SIZE + 1];
int packetByteCount = 0;
int packetIValue;   // int value
unsigned int packetUIValue;  // unsigned int value
long packetLValue;   // long value
unsigned long packetULValue;   // unsigned long value

void readTp() {
  while (Serial1.available() > 0) {
    byte b = Serial1.read();
    if (b & 0x80) { // command?
      if (b < CMD_PARAM) { // Single-byte command?
        doSingleCmd(b);
      }
      else {
        doParamCmd(b);
      }
    }
    else {
      if (packetByteCount < MAX_PACKET_SIZE) {
        packetBytes[packetByteCount++] = b;
      }
    }
  } // end while()
}

void doSingleCmd(byte cmd) {
  boolean flag =cmd & CMD_SINGLE_FLAG;
  cmd = cmd & (CMD_SINGLE_FLAG ^ 0xFF);  // mask of flag bit

  switch (cmd) {
  case CMD_LED: // passed on from PC controller
    if (flag) {
      digitalWrite(PIN_LED, HIGH);
    } else {
      digitalWrite(PIN_LED, LOW);
    }
    break;
  default:
    unknownSingleCmd++;
    break;
  } // end switch(cmd)
}


void doParamCmd(byte cmd) {
  switch (cmd) { 
  case CMD_RUN_STATE_STAT:
    if (packetU1()) {
      runState = packetUIValue;
    } 
    break;
  case CMD_VAL_SET_STAT:
    if (packetU1()) {
      valSetStat = packetUIValue;
    } 
    break;
  case CMD_MODE_STAT:
    if (packetU1()) {
      mode = packetUIValue;
    } 
    break;
  case CMD_BATTVOLT_VAL:
    if (packetU2()) {
      tpBatteryVolt = packetUIValue * 0.01f;
    } 
    break;
  case CMD_FROM_TP_PING:
    if (packetU1()) {
      pingTpTime = timeMilliseconds;
      if (pingHcCount != packetUIValue) { // same value we sent out?
        pingTpErrors++;
      }
    } 
    break;
  default:
    unknownParamCmd++;
    break;
  }
  packetByteCount = 0;
}

// Unsigned int, 1 byte
boolean packetU1() {
  if (packetByteCount != 1) {
    byteCountErrors++;
    return false;
  }
  packetUIValue = packetBytes[0];
  return true;
}

//  int, 1 byte
boolean packet1() {
  boolean ret = packetU1();
  packetIValue = ((int) packetUIValue) - 64;
  return ret;
}

// unsigned int, 2 bytes
boolean packetU2() {
  if (packetByteCount != 2) {
    byteCountErrors++;
    return false;
  }
  packetUIValue = packetBytes[0];
  packetUIValue += packetBytes[1] << 7;
  return(true);
}

// int, 2 bytes
boolean packet2() {
  boolean ret = packetU2();
  packetIValue = ((int) packetUIValue) - 8192;
  return(ret);
}

// unsigned int, 4 bytes
boolean packetU4() {
  if (packetByteCount != 4) {
    byteCountErrors++;
    return false;
  }    
  unsigned long b1 = packetBytes[0];
  unsigned long b2 = packetBytes[1];
  unsigned long b3 = packetBytes[2];
  unsigned long b4 = packetBytes[3];
  packetULValue = b1;
  packetULValue += b2 * 128; // left shift operator doesn't work!
  packetULValue += b3 * 128* 128;
  packetULValue += b4 * 128* 128* 128;
  return(true);
}

// int, 4 bytes
boolean packet4() {
  boolean ret = packetU4();
  packetLValue = ((long) packetULValue) - 268435456L;
  return(ret);
}

boolean packetMsg() {
  packetBytes[packetByteCount] = 0;
  return true;
}
