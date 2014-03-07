
void cmdSingle(byte cmd, boolean b) {
  if (b == true) {
    cmd |= CMD_SINGLE_FLAG;
  }
  Serial1.write(cmd);
}

void cmdOneU(int cmd, unsigned int val) {
  if (val >= 128) {
    val = 127;
  } 
  else if (val < 0) {
    val = 0;
  }
  byte lb = ((byte) val) & 0x7F;
  Serial1.write(lb);
  Serial1.write(cmd);
}

void cmdOne(int cmd, int val) {
  cmdOneU(cmd, val + 64);
}

void cmdTwoU(int cmd, unsigned int val) {
  if (val >= 16384) {
    val = 16383;
  } 
  else if (val < 0) {
    val = 0;
  }
  byte lb = ((byte) val) & 0x7F;
  byte hb = (val >> 7) & 0x7F;
  Serial1.write(lb);
  Serial1.write(hb);
  Serial1.write(cmd);
}

void cmdTwo(int cmd, int val) {
  cmdTwoU(cmd, val + 8192);
}

void cmdFourU(int cmd, unsigned long val) {
  byte b1 = val & 0x7F;
  val = val >> 7;
  byte b2 = val & 0x7F;
  val = val >> 7;
  byte b3 = val & 0x7F;
  val = val >> 7;
  byte b4 = val & 0x7F;
  Serial1.write(b1);
  Serial1.write(b2);
  Serial1.write(b3);
  Serial1.write(b4);
  Serial1.write(cmd);
}

void cmdFour(int cmd, long val) {
  cmdFourU(cmd, val + 268435456L);
}


void cmdMsg(char* str) {
  Serial1.print(str);
  Serial1.write(CMD_MSG);
}




/***************************************************************
 *
 * debugXXX()
 *
 *       Send debug to the serial usb port
 *
 ***************************************************************/
void debugByte(char msg[], byte val) {
  Serial.print(msg);
  Serial.println(val, HEX);
}

void debugStr(char msg1[], char msg2[]) {
  Serial.print(msg1);
  Serial.println(msg2);
}

void debugFloat(char msg[], float f) {
  Serial.print(msg);
  Serial.println(f);
}

void debugInt(char msg[], int i) {
  Serial.print(msg);
  Serial.println(i);
}

void debugLong(char msg[], long i) {
  Serial.print(msg);
  Serial.println(i);
}

void debugBoolean(char msg[], boolean b) {
    Serial.print(msg);
    if (b) {
      Serial.println("true");
    }
    else {
      Serial.println("false");
    }
}




