unsigned int updateRow = 0;
int tpBattVoltDisp = 0;
boolean isConnectedDisp1 = true;
boolean isConnectedDisp2 = false;
boolean isConnectedDisp3 = false;

// Initialize the LCD screen
void lcdInit() {
  if (digitalRead(PIN_SW1L) == LOW) { // need to reset things?
    lcdSerial.begin(19200);
    delay(1200);
    lcdSerial.write(124);
    lcdSerial.write(128);  // Set backlight (128-157)
    delay(500);
    powerDown();  // Turn off power.
  }
  if (digitalRead(PIN_SW1R) == LOW) { // need to reset things?
    lcdSerial.begin(19200);
    delay(1200);
    lcdSerial.write(124);
    lcdSerial.write(140);  // Set backlight (128-157)
    delay(500);
    powerDown();  // Turn off power.
  }

  if (digitalRead(PIN_SW2L) == LOW) { // need to reset things?
    delay(100);
    
    // Resetting as in next 2 lines doesn't work -- too late?
    //lcdSerial.begin(9600);  // Put chosen rate here.
    //lcdSerial.write(18); // CTRL-R sets to 9600
  
    // May need to fiddle with next lines if baud rate has been lost.
    // Be sure the processor is set to 3.3V 8MHz!
    delay(1100);
    lcdSerial.begin(19200);
    //lcdSerial.write(124); 
    //lcdSerial.write(15);   // 2400=11, 4800=12, 9600=13, 14400=14, 19200=15, 38400=16  
    //delay(100);
    //lcdSerial.begin(19200);
    
    
    delay(1100);
    //  Uncomment the following lines (only!) system settings are corrupt.
    //lcdSerial.write(124);
    //lcdSerial.write(3);  // Sets 20 characters wide
    //lcdSerial.write(124);
    //lcdSerial.write(5);  // Sets 4 lines tall
    
    //  Uncomment the following lines (only!) to change the splash.
    //cursor(0,0);
    //lcdSerial.print("     TwoPotatoe     ");
    //cursor(0, 1);
    //lcdSerial.print("   Hand Controller  ");
    //lcdSerial.write(124);
    //lcdSerial.write(10);

  }
  delay(1200);  // Why do we need such a long delay?
  lcdSerial.begin(19200);  // Put chosen rate here.
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
  switch (updateRow++ % 4) {
  case 0:
    updateRow1();
    break;
  case 1:
    updateRow2();
    break;
  case 2:
    updateRow3();
    break;
  case 3:
    lcdPrintStatusLine();
    break;
  }
}


/*****************************************************
 *    updateRow1() 
 ****************************************************/
void updateRow1() {
  if (!isConnected) {
    if (isConnectedDisp1 == true) {
      isConnectedDisp1 = false;
      tpBattVoltDisp = 0;
      tpStateDisp = -1;
      cursor(0,0);
      lcdSerial.print("---No Connection----");
    }
  }
  else {
    isConnectedDisp1 = true;
    lcdTpBattery();
    if (tpStateDisp != tpState) {
      tpStateDisp = tpState;
      cursor(0,0);
      if ((tpState & TP_STATE_RUN_READY) == 0) {
        lcdSerial.print("Idle      ");
      }
      else if ((tpState & TP_STATE_RUNNING) != 0){
        lcdSerial.print("Running   ");
      }
      else if ((tpState & TP_STATE_UPRIGHT) == 0){
        lcdSerial.print("Fallen    ");
      }
      else if ((tpState & TP_STATE_ON_GROUND) == 0){
        lcdSerial.print("Lifted    ");
      }
      else {
        lcdSerial.print("????      ");
      }
    }
  }
} // end updateRow1() 


/*****************************************************
 *    updateRow2() 
 ****************************************************/
void updateRow2() {
  if (!isConnected) {
    lcdLocalBattery();
    if (isConnectedDisp2 == true) {
      isConnectedDisp2 = false;
      tpModeDisp = -1;
      cursor(0,1);
      lcdSerial.print("          ");
    }
  }
  else {
    isConnectedDisp2 = true;
    lcdLocalBattery();
    if (tpModeDisp != tpMode) {
      cursor(0,1);
      switch (tpMode) {
      case MODE_TP5:
        lcdSerial.print("Mode: TP5 ");
        break;
      case MODE_TP6:
        lcdSerial.print("Mode: TP6 ");
        break;
      case MODE_DRIVE:
        lcdSerial.print("MD: DRIVE ");
        break;
      default:
        lcdSerial.print("Unknown MD");
        break;
      }
    }
  }
} // end updateRow2() 


/*****************************************************
 *    updateRow3() 
 ****************************************************/
void updateRow3() {
  if (!isConnected) {
    if (isConnectedDisp3 == true) {
      isConnectedDisp3 = false;
      tpModeDisp = -1;
      cursor(0,2);
      lcdSerial.print("                    ");
    }
  }
  else {
    isConnectedDisp3 = true;
    if (tpModeDisp != tpMode) {
      cursor(0,2);
      switch (tpValSet) {
      case VAL_SET_A:
        lcdSerial.print("Set: A    ");
        break;
      case VAL_SET_B:
        lcdSerial.print("Set: B    ");
        break;
      case VAL_SET_C:
        lcdSerial.print("Set: C    ");
        break;
      default:
        lcdSerial.print("          ");
        break;
      }
    }
  }
} // end updateRow3() 



void lcdLocalBattery() {
  float volt = getLocalBattery();
  if (volt < 7.0f) {
    lowBattery();
  }
//  a * 0.0092f;
  printVolt(10, 1, volt);
  printPct(16, 1, false, volt);
}

float getLocalBattery() {
  int a = analogRead(PIN_BATT);
  return ((float) a) * .009;    
}

void lowBattery() {
  clearScreen();
  cursor(0,0);
  lcdSerial.print("--- Low Battery ----");
  delay(100);
  while(getLocalBattery() < 7.0) {
    delay(100);
  }
}

void lcdTpBattery() {
  if (tpBattVoltDisp != tpBattVolt) { 
    tpBattVoltDisp = tpBattVolt;
    float volt = ((float) tpBattVolt) * 0.01;
    printVolt(10, 0, volt);
    printPct(16, 0, true, volt);
  }
}

void printVolt(int x, int y, float volt) {
  cursor(x, y);
  if (volt < 10.00) {
    lcdSerial.print(" ");
  }
  lcdSerial.print(volt);
  lcdSerial.print("V");
}

void printPct(int x, int y, boolean src, float volt) {
  cursor(x, y);
  int pct = (int) getPct(src, volt);
  if (pct < 10) {
    lcdSerial.print("  ");
  }
  else if (pct < 100) {
    lcdSerial.print(" ");
  } 
  lcdSerial.print(pct);
  lcdSerial.print("%");
}


void lcdValSet() {
}

float tpVolt[] = { 
  12.6f, 11.99f, 11.75f, 11.54f, 11.23f, 11.05f, 10.83f, 10.63f, 10.35f, 9.0f};
float tpPct[] =  {
  100.0f, 83.0f,  71.0f,  60.0f,  34.0f,  20.0f,  07.0f,  05.0f,  03.0f,  0.0f};
float localVolt[] = {  
  8.4f,  8.09f, 7.57f, 7.34f, 7.12f, 6.0f};
float localPct[] =  {
  100.0f, 88.0f, 49.0f,  20.0f, 5.0f,  0.0f};

// Convert battery voltage to percent                  
float getPct(boolean src, float batteryVolt) {
  float* volt;
  float* pct;
  int voltSize;
  if (src) {
    volt = tpVolt;
    pct = tpPct;
    voltSize = sizeof(tpVolt)/sizeof(float); 
  }
  else {
    volt = localVolt;
    pct = localPct;
    voltSize = sizeof(localVolt)/sizeof(float); 
  }

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
      float voltPct = (batteryVolt - volt[i])/rangeVolt;
      return pct[i] +(voltPct * rangePct);
    }
  }
  return-99.0f;
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

// Save number in array of 5 columns of 4 digits each.
void lcdDebug(int num, int column) {
  if ((column < 0) || (column > 4)) {
    return;
  }
  if (num > 999) {
    row4Values[column] = -2; // Error
  }
  else if (num < 0) {
    row4Values[column] = -1; // Blank
  }
  else {
    row4Values[column] = num; 
  }
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
  cursor(0,0);
  lcdSerial.print("     TwoPotatoe2    ");
  cursor(0, 1);
  lcdSerial.print("   Hand Controller  ");
  lcdSerial.write(124);
  lcdSerial.write(10);
}


