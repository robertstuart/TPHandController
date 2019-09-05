/*****************************************************************************-
                                LCD
                        upcate the LCD display
 *****************************************************************************/
//boolean isp2ModeDisp = true;
boolean isConnectedDisp = false;
float hcBattDisp = 0;
float botBattDisp = 0.0;
int stateDisp = 0;
float fpsDisp = 0.0;
int tuningDisp = 99;
String messageDisp = "";
float v1Disp = 0.0;
float v2Disp = 0.0;
String isFastModeDisp = "Slow";
int stepDisp = 0;

String spacex = String("                    ");



/*****************************************************************************-
    lcdUpdate()   called periodically to update the display
 *****************************************************************************/
void lcdUpdate() {
  static boolean oldIsP2Mode = true;
  static int loop = 0;
  
  loop++;
  loop = loop % 20;
  if (!loop) getHcBatt();

  // Refresh if mode changes.
  if (oldIsP2Mode != isP2Mode) {
    oldIsP2Mode = isP2Mode;
    setConnectedState();
    return;
  } 
  
  // Check on all P2 display variables.
  if (isP2Mode) {
    if (isConnectedDisp != is2Connected) {
      setConnectedState();
    } else {
      if (stateDisp != p2State) setState();
      if (botBattDisp != p2BattV)  setBotBatt();
      if (fpsDisp != p2Fps) setFps();
      if (tuningDisp != p2Tuning) setTuning();
      if (!messageDisp.equals(p2Message)) setMessage(p2Message);
//      if (v1Disp != p2V1) setV1();
//      if (v2Disp != p2V2) setV2();
//      if (stepDisp != p2Step) setStep();
    }
  } else {
    if (isConnectedDisp != is6Connected) {
      setConnectedState();
    } else {
      if (stateDisp != p6State) setState();
      if (botBattDisp != p6BattV)  setBotBatt();
      if (fpsDisp != p6Fps) setFps();
      if (tuningDisp != p6Tuning) setTuning();
      if (messageDisp.equals(p6Message)) setMessage(p6Message);
//      if (v1Disp != p6V1) setV1();
//      if (v2Disp != p6V2) setV2();
//      if (stepDisp != p6Step) setStep();
    }
  }
  if (hcBattDisp != hcBattV) setHcBatt();
}



/*****************************************************************************-
    setConnectedState()  Called whenever the current connected state changes
                         or isP2Mode changes.
 *****************************************************************************/
void setConnectedState() {
  clearScreen();
  cursor(0, 0);
  isConnectedDisp = (isP2Mode) ? is2Connected : is6Connected;
  if (isConnectedDisp) {
    if (isP2Mode) Serial3.print("TwoPotatoe:         ");
    else          Serial3.print("SixPotatoe:         ");
    // Set disp variables so they are all refreshed.
    stateDisp = 0;
    botBattDisp = -1.44;
    fpsDisp = 999.9;
    tuningDisp = 99;
    messageDisp = "";
    v1Disp = 1234.1224;
    v2Disp = 1234.1224;
    stepDisp = 9999;
  } else {
    if (isP2Mode) Serial3.print("---TwoPotatoe off---");
    else        Serial3.print("---SixPotatoe off---");
    // Set disp variables so they don't refresh
    stateDisp = (isP2Mode) ? p2State : p6State;
    botBattDisp = (isP2Mode) ? p2BattV : p6BattV;
    fpsDisp = (isP2Mode) ? p2Fps : p6Fps;
    tuningDisp = (isP2Mode) ? p2Tuning : p6Tuning;
    messageDisp = "                    ";
    v1Disp = (isP2Mode) ? p2V1 : p6V1;
    v2Disp = (isP2Mode) ? p2V2 : p6V2;
    stepDisp = (isP2Mode) ? p2Step : p2Step;
  }
}



/*****************************************************************************-
    setState()
 *****************************************************************************/
void setState() {
  String stateStr;
  stateDisp = (isP2Mode) ? p2State : p6State;
    cursor(12, 0);
    if (is2Running) {
    if (!is2Upright) {
      stateStr = "Fall ";
    } else if (is2RouteInProgress) {
      stateStr = "Route ";
    } else {
      stateStr = "Run! ";
    }
  } else {
    stateStr = "Idle ";
  }
  Serial3.print(stateStr);
}



/*****************************************************************************-
 *  setBotBatt() Stet the robot battery pct.
 *****************************************************************************/
void setBotBatt() {
  if (isP2Mode) {
    botBattDisp = p2BattV;
    setBattPct(0, botBattDisp, 6, (char *) " ");
  } else {
    botBattDisp = p6BattV;
    setBattPct(0, botBattDisp, 3, (char *) " ");
  }
//  Serial.println(botBattDisp);
}


/*****************************************************************************-
    setFps()
 *****************************************************************************/
void setFps() {
  fpsDisp = (isP2Mode) ? p2Fps : p6Fps;
  sprintf(message, "%5.1f fps", fpsDisp);
  cursor(0, 1);
  Serial3.print(message);
}



/*****************************************************************************-
    setTuning()
 *****************************************************************************/
void setTuning() {
  String s;
  tuningDisp =  (isP2Mode) ? p2Tuning : p6Tuning;;
  cursor(11, 1);
  if (tuningDisp == 0) s = "Slow";
  else if (tuningDisp == 1) s = "Med";
  else s = "Fast";
  Serial3.print(s);
}

void setMessage(String msg) {
  messageDisp = msg;
  cursor(0, 3);
  Serial3.print(msg);
}
//
//
//
///*****************************************************************************-
//    setV1()
// *****************************************************************************/
//void setV1() {
//  v1Disp = (isP2Mode) ? p2V1 : p6V1;
//  cursor(0, 3);
//  sprintf(message, "%7.2f", v1Disp);
//  Serial3.print(message);
//}



///*****************************************************************************-
//    setV2()
// *****************************************************************************/
//void setV2() {
//  v2Disp = (isP2Mode) ? p2V2 : p6V2;
//  cursor(8, 3);
//  sprintf(message, "%7.2f", v2Disp);
//  Serial3.print(message);
//}



/*****************************************************************************-
    setStep()
 *****************************************************************************/
//void setStep() {
//  stepDisp = (isP2Mode) ? p2Step : p6Step;
//  cursor(8, 3);
//  sprintf(message, "%7.2f", v2Disp);
//  Serial3.print(message);
//}



/*****************************************************************************-
    hcBatt()
 *****************************************************************************/
void setHcBatt() {
    cursor(18, 3);
    setBattPct(3, hcBattV, 2, (char *) "h");
}



void setBattPct(int y, float volt, int cells, char trail[]) {
  float newVolt =  volt / ((float) cells);
  float pct = getPct(newVolt);
  cursor(17, y);
  sprintf(message, "%02d", (int) pct);
  Serial3.print(message);
  Serial3.print(trail);
  //  setIntVal(1, y, 2, (int) pct, trail);
}

float getHcBatt() {
  hcBattV = (float) analogRead(PIN_BATT) * 0.00911;;
  return hcBattV;
}

float battVolt[] = {
  4.2f, 4.0f, 3.92f, 3.85f, 3.74f, 3.68f, 3.61f, 3.54f, 3.45f, 3.0f
};
float battPct[] =  {
  99.0f, 83.0f,  71.0f,  60.0f,  34.0f,  20.0f,  07.0f,  05.0f,  03.0f,  0.0f
};

// Convert battery voltage to percent
float getPct(float batteryVolt) {
  //  float batteryVolt = ((float) iVolt) / 100;
  float* volt;
  float* pct;
  int voltSize;

  volt = battVolt;
  pct = battPct;
  voltSize = sizeof(battVolt) / sizeof(float);

  if (volt[0] < batteryVolt) {
    return 99.0f;
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
  Serial3.write(0xFE);
  Serial3.write(base);
}

void clearScreen() {
  Serial3.write(0xFE);
  Serial3.write(0x01);
}



void setSplash() {
  cursor(0, 0);
  Serial3.print("     TwoPotatoe     ");
  cursor(0, 1);
  Serial3.print("   Hand Controller  ");
  Serial3.write(124);
  Serial3.write(10);
}



/*****************************************************************************-
    lcdInit()   Initialize the display
 *****************************************************************************/
void lcdInit() {
  //  if (digitalRead(PIN_SW1L) == LOW) { // need to reset things?
  //    Serial3.begin(19200);
  //    delay(1200);
  //    Serial3.write(124);
  //    Serial3.write(128);  // Set backlight (128-157)
  //    delay(500);
  //    powerDown();  // Turn off power.
  //  }
  //  if (digitalRead(PIN_SW1R) == LOW) { // need to reset things?
  //    Serial3.begin(19200);
  //    delay(1200);
  //    Serial3.write(124);
  //    Serial3.write(140);  // Set backlight (128-157)
  //    delay(500);
  //    powerDown();  // Turn off power.
  //  }
  //
  //  if (digitalRead(PIN_SW2L) == LOW) { // need to reset things?
  //    delay(100);
  //
  //    // Resetting as in next 2 lines doesn't work -- too late?
  //    //Serial3.begin(9600);  // Put chosen rate here.
  //    //Serial3.write(18); // CTRL-R sets to 9600
  //
  //    // May need to fiddle with next lines if baud rate has been lost.
  //    delay(1100);
  //    Serial3.begin(19200);
  //    //Serial3.write(124);
  //    //Serial3.write(15);   // 2400=11, 4800=12, 9600=13, 14400=14, 19200=15, 38400=16
  //    //delay(100);
  //    //Serial3.begin(19200);
  //
  //
  //    delay(1100);
  //    //  Uncomment the following lines (only!) system settings are corrupt.
  //    //Serial3.write(124);
  //    //Serial3.write(3);  // Sets 20 characters wide
  //    //Serial3.write(124);
  //    //Serial3.write(5);  // Sets 4 lines tall
  //
  //    //  Uncomment the following lines (only!) to change the splash.
  //    //cursor(0,0);
  //    //Serial3.print("     TwoPotatoe     ");
  //    //cursor(0, 1);
  //    //Serial3.print("   Hand Controller  ");
  //    //Serial3.write(124);
  //    //Serial3.write(10);
  //
  //  }
  delay(900);  // Why do we need such a long delay?
  Serial3.begin(9600);  // Put chosen rate here.
  clearScreen();
  delay(200);  // Why is this necessary?
}
