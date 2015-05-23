void sendJoyXY() {
  sendXMsg(RCV_JOYX, 2, joyX);
  sendXMsg(RCV_JOYY, 2, joyY);
}



void sendXMsg(int cmd, int precision, double val) {
  Serial1.write(cmd);
  Serial1.print(val, precision);
  Serial1.write((byte) 0);
}

void sendXMsg(int cmd, int val) {
  Serial1.write(cmd);
  Serial1.print(val);
  Serial1.write((byte) 0);
}

void sendXMsg(int cmd, String val) {
  Serial1.write(cmd);
  Serial1.print(val);
  Serial1.write((byte) 0);
}

