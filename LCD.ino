unsigned int updateRow = 0;
int tpBattVoltDisp = 0;
boolean isConnectedDisp = true;
int debug1Disp = INT_MIN;
int debug2Disp = INT_MIN;
int tpStateDisp = 0;
int tpModeDisp = MODE_UNKNOWN;
int tpValSetDisp = 0;
int tpBMBattDisp = 0;
int tpEMBattDisp = 0;
int tpLBattDisp = 0;
int hcBattDisp = 0;
int tpPitchDisp = 0;
int tpFpsDisp = 0;

// Initialize the LCD screen
void lcdInit() {
//  if (digitalRead(PIN_SW1L) == LOW) { // need to reset things?
//    lcdSerial.begin(19200);
//    delay(1200);
//    lcdSerial.write(124);
//    lcdSerial.write(128);  // Set backlight (128-157)
//    delay(500);
//    powerDown();  // Turn off power.
//  }
//  if (digitalRead(PIN_SW1R) == LOW) { // need to reset things?
//    lcdSerial.begin(19200);
//    delay(1200);
//    lcdSerial.write(124);
//    lcdSerial.write(140);  // Set backlight (128-157)
//    delay(500);
//    powerDown();  // Turn off power.
//  }
//
//  if (digitalRead(PIN_SW2L) == LOW) { // need to reset things?
//    delay(100);
//
//    // Resetting as in next 2 lines doesn't work -- too late?
//    //lcdSerial.begin(9600);  // Put chosen rate here.
//    //lcdSerial.write(18); // CTRL-R sets to 9600
//
//    // May need to fiddle with next lines if baud rate has been lost.
//    // Be sure the processor is set to 3.3V 8MHz!
//    delay(1100);
//    lcdSerial.begin(19200);
//    //lcdSerial.write(124);
//    //lcdSerial.write(15);   // 2400=11, 4800=12, 9600=13, 14400=14, 19200=15, 38400=16
//    //delay(100);
//    //lcdSerial.begin(19200);
//
//
//    delay(1100);
//    //  Uncomment the following lines (only!) system settings are corrupt.
//    //lcdSerial.write(124);
//    //lcdSerial.write(3);  // Sets 20 characters wide
//    //lcdSerial.write(124);
//    //lcdSerial.write(5);  // Sets 4 lines tall
//
//    //  Uncomment the following lines (only!) to change the splash.
//    //cursor(0,0);
//    //lcdSerial.print("     TwoPotatoe     ");
//    //cursor(0, 1);
//    //lcdSerial.print("   Hand Controller  ");
//    //lcdSerial.write(124);
//    //lcdSerial.write(10);
//
//  }
  delay(1200);  // Why do we need such a long delay?
  lcdSerial.begin(9600);  // Put chosen rate here.
  delay(100);
  clearScreen();
  delay(100);
}


/*****************************************************
 *
 *    lcdUpdate()
 *
 *         Only called during "idle" periods so the
 *         SoftwareSerial does not interfere with
 *         activity on the UART.
 *
 ****************************************************/
void lcdUpdate() {
  lcdUpdateTrigger = timeMilliseconds + 100;
  if (isConnectedDisp != isConnected) {
    updateConnected();
  }
  else if (debug1Disp != debug1) {
    debug1Disp = debug1;
    setIntVal(0, 3, 6, debug1, " ");
  }
  else if (debug2Disp != debug2) {
    debug2Disp = debug2;
    setIntVal(7, 3, 6, debug2, " ");
  }
  else if (tpStateDisp != tpState) {
    setState();
  }
  else if (tpModeDisp != tpMode) {
    setMode();
  }
  else if (tpValSetDisp != tpValSet) {
    setValSet();
  }
  else if (tpBMBattDisp != tpBMBatt) {
    tpBMBattDisp = tpBMBatt;
    setBattPct(0, tpBMBatt, 1.0, "b", false);
  }
  else if (tpEMBattDisp != tpEMBatt) {
    tpEMBattDisp = tpEMBatt;
    setBattPct(1, tpEMBatt, 1.5, "e", true);
  }
  else if (tpLBattDisp != tpLBatt) {
    tpLBattDisp = tpLBatt;
    setBattPct(2, tpLBatt, 1.5, "l", true);
  }
  else if (tpPitchDisp != tpPitch) {
    setPitch();
  }
  else if (tpFpsDisp != tpFps) {
    setFps();
  }
  else if (hcBattDisp != getHcBatt()) {
    hcBattDisp = hcBatt;
    setBattPct(3, hcBatt, 1.5, "h", true);
  }
}

void updateConnected() {
  isConnectedDisp = isConnected;
  if (!isConnected) {
    cursor(0, 0);
    lcdSerial.print("---No Connection----");
    tpState = tpMode = tpValSet = tpFps = tpPitch = INT_MIN;
    tpBMBatt = tpEMBatt = tpLBatt = INT_MIN;
  }
}

void setState() {
  char* stateStr;
  tpStateDisp = tpState;
  cursor(0, 0);
  if (tpState != INT_MIN) {
    if (isStateBitSet(TP_STATE_RUNNING)) {
      stateStr = "Run! ";
    }
    else if (isStateBitSet(TP_STATE_RUN_READY)) {
      if (isStateBitClear(TP_STATE_UPRIGHT)) stateStr = "Fall ";
      else stateStr = "Lift ";
    }
    else {
      stateStr = "Idle ";
    }  
    lcdSerial.print(stateStr);
  }
}

void setMode() {
  char* modeStr = "";
  tpModeDisp = tpMode;
  if (tpMode != INT_MIN) {
    cursor(5, 0);
    switch (tpMode) {
      case MODE_TP5:
        modeStr = "TP5 ";
        break;
      case MODE_TP6:
        modeStr = "TP6 ";
        break;
      default:
        modeStr = "TP? ";
        break;
    }
    lcdSerial.print(modeStr);
  }
}

void setValSet() {
  char* valSetStr = "";
  tpValSetDisp = tpValSet;
  if (tpValSet != INT_MIN) {
    cursor(9, 0);
    switch (tpValSet) {
      case 0:
        valSetStr = "SetA    ";
        break;
      case 1:
        valSetStr = "SetB    ";
                 break;
      case 2:
        valSetStr = "SetC    ";
                 break;
      default:
        valSetStr = "Set?    ";
                 break;
    }
    lcdSerial.print(valSetStr);
  }
}

void setFps() {
  tpFpsDisp = tpFps;
  if (tpFps == INT_MIN) {
    cursor(0, 1);
    lcdSerial.print("        ");
  }
  else {
    setFloatVal(0, 1, 3, tpFps, "fps ");
  }
}

void setPitch() {
  char fill[] = {223, 32, 32, 32, 0};
  tpPitchDisp = tpPitch;
  if (tpPitch == INT_MIN) {
    cursor(8, 1);
    lcdSerial.print("         ");
  }
  else {
    setFloatVal(8, 1, 4, tpPitch, fill); //º
  }
}

void setIntVal(int x, int y, int width, int val, char* trail) {
  char spaces[] = "          ";
  int spaceCount;
  cursor(x, y);
  if (val >= 10000) spaceCount = 1;
  else if (val >= 1000) spaceCount = 0;
  else if (val >= 100) spaceCount = 1;
  else if (val >= 10) spaceCount = 2;
  else if (val >= 0) spaceCount = 3;
  else if (val > -10) spaceCount = 2;
  else if (val > -100) spaceCount = 1;
  else if (val > -1000) spaceCount = 0;
  else if (val > -10000) spaceCount = 0;
  else spaceCount = 0;
  spaceCount = (spaceCount - 4) + width;
  if (spaceCount < 0) spaceCount = 0;
  lcdSerial.write(spaces, spaceCount);
  lcdSerial.print(val);
  lcdSerial.print(trail);
}

// "val" is and int 100 x
void setFloatVal(int x, int y, int width, int val, char trail[]) {
  char spaces[] = "          ";
  int spaceCount;
  float fVal = ((float) val) / 100.0;
  if (fVal >= 10.0) spaceCount = 1;
  else if (fVal >= 0.0) spaceCount = 2;  // >=100.0
  else if (fVal > -10.0) spaceCount = 1;   // >= 10.0
  else  spaceCount = 0;      // >= 0.0
  spaceCount = (spaceCount - 4) + width;
  if (spaceCount < 0) spaceCount = 0;
  cursor(x, y);
  lcdSerial.write(spaces, spaceCount);
  float v = ((float) val) / 100.0;
  lcdSerial.print(v, 1);
  lcdSerial.print(trail);
//  if (y == 
}

void setBattPct(int y, int volt, float factor, char trail[], boolean clr) {
  cursor(16, y);
  if (volt == INT_MIN) {
    if (clr) lcdSerial.print("    ");
  }
  else {
    int newVolt = (int) (((float) volt) * factor);
    float pct = getPct(newVolt);
    setIntVal(16, y, 3, (int) pct, trail);
  }
}

int getHcBatt() {
  int a = analogRead(PIN_BATT);
debug1 = 942;
  hcBatt = (int) ((1000L * a) / 1120L);
  lowBattery();
  return hcBatt;
}

void lowBattery() {
  if (hcBatt < 630) {
    if (timeMilliseconds < timeLowBatt) timeLowBatt = timeMilliseconds;
    cursor(0, 2);
    lcdSerial.print("--- Low Battery ----");
    if ((timeMilliseconds - timeLowBatt) > 100000L) powerDown(); // 100 seconds.
  }
}

float tpVolt[] = {
  12.6f, 11.99f, 11.75f, 11.54f, 11.23f, 11.05f, 10.83f, 10.63f, 10.35f, 9.0f
};
float tpPct[] =  {
  100.0f, 83.0f,  71.0f,  60.0f,  34.0f,  20.0f,  07.0f,  05.0f,  03.0f,  0.0f
};
float localVolt[] = {
  8.4f,  8.09f, 7.57f, 7.34f, 7.12f, 6.0f
};
float localPct[] =  {
  100.0f, 88.0f, 49.0f,  20.0f, 5.0f,  0.0f
};

// Convert battery voltage to percent
float getPct(int iVolt) {
  float batteryVolt = ((float) iVolt) / 100;
  float* volt;
  float* pct;
  int voltSize;

  volt = tpVolt;
  pct = tpPct;
  voltSize = sizeof(tpVolt) / sizeof(float);

  if (volt[0] < batteryVolt) {
    return 100.0f;
  }
  if (volt[voltSize - 1] > batteryVolt) {
    return 0.0f;
  }
  for (int i = 1; i < voltSize ; i++) {
    if (volt[i] < batteryVolt) {
      float rangeVolt = volt[i - 1] - volt[i];
      float rangePct = pct[i - 1] - pct[i];
      float voltPct = (batteryVolt - volt[i]) / rangeVolt;
      return pct[i] + (voltPct * rangePct);
    }
  }
  return -99.0f;
}

/**************************** LCD commands ***********************/
void cursor(int x, int y)  { // origin is 0,0
  int base;
  switch (y) {
    case 0:
      base = 0;
      break;
    case 1:
      base = 64;
      break;
    case 2:
      base = 20;
      break;
    case 3:
      base = 84;
      break;
    default:
      base = 0;
  }
  base += 128 + x;
  lcdSerial.write(0xFE);
  lcdSerial.write(base);
}

void clearScreen() {
  lcdSerial.write(0xFE);
  lcdSerial.write(0x01);
}


// Print the status line.
void lcdPrintStatusLine() {
  for (int i = 0; i < 5; i++) {
    if (row4Values[i] != row4ValuesDisplay[i]) {
      int value = row4Values[i];
      row4ValuesDisplay[i] = value;
      cursor(4 * i, 3);
      if (value == -1) {
        lcdSerial.print("    ");
      }
      else if (value == -2) {
        lcdSerial.print(" ERR");
      }
      else {
        if (value < 10) {
          lcdSerial.print("   ");
        }
        else if (value < 100) {
          lcdSerial.print("  ");
        }
        else {
          lcdSerial.print(" ");
        }
        lcdSerial.print(value);
      }
    }
  }
}

void setSplash() {
  cursor(0, 0);
  lcdSerial.print("     TwoPotatoe2    ");
  cursor(0, 1);
  lcdSerial.print("   Hand Controller  ");
  lcdSerial.write(124);
  lcdSerial.write(10);
}


