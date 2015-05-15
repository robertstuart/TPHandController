unsigned int updateRow = 0;
int tpBattVoltDisp = 0;
int sonarDistanceDisp = 0;
int yawDisp = 0;
boolean isConnectedDisp = true;
int debug1Disp = NO_DISP;
int debug2Disp = NO_DISP;
int tpStateDisp = 0;
int tpHeadingDisp = 0;
int tpModeDisp = 0;
int tpValSetDisp = 0;
int tpBattDisp = 0;
int hcBattDisp = 0;
int tpPitchDisp = 0;
int tpFpsDisp = 0;
boolean tpLiftDisp = 0;

// Initialize the LCD screen
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
//    // Be sure the processor is set to 3.3V 8MHz!
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
  static int loop = 0;
  loop = ++loop % 10;
  lcdUpdateTrigger = timeMilliseconds + 100;
  
  // The values in order of appearance.
  if (isConnectedDisp != isConnected) {
    updateConnected();
  }
  if (tpStateDisp != tpState) {
    setState();
  }
  if (tpModeDisp != tpMode) {
    setMode();
  }
  if (tpValSetDisp != tpValSet) {
    setValSet();
  }
  if ((loop == 7) && (tpBattDisp != tpBatt)) {
    tpBattDisp = tpBatt;
    if (tpBatt == NO_DISP) return;
    setBattPct(0, tpBatt, 0.75, "b");
  }
  else if (tpFpsDisp != tpFps) {
    tpFpsDisp = tpFps;
    setFloatVal(0, 1, 3, tpFps, "fps ");
  }
  else if (tpPitchDisp != tpPitch) {
    char fill[] = {223, 0};
    tpPitchDisp = tpPitch;
    setFloatVal(8, 1, 4, tpPitch, fill); //º
  }
  if (sonarDistanceDisp != sonarDistance) {
    sonarDistanceDisp = sonarDistance;
    setFloatVal(0, 2, 4, sonarDistance, "ft"); //
  }
  if (tpHeadingDisp != tpHeading) {
    char fill[] = {223, 0};
    tpHeadingDisp = tpHeading;
    setIntVal(8, 2, 5, tpHeading, fill); //
  }
  if (debug1Disp != debug1) {
    debug1Disp = debug1;
    setIntVal(13, 2, 6, debug1, " ");
  }
//  else if (debug2Disp != debug2) {
//    debug2Disp = debug2;
//    setIntVal(7, 3, 6, debug2, " ");
//  }
  if ((loop == 2) && (hcBattDisp != getHcBatt())) {
    hcBattDisp = hcBatt;
    setBattPct(3, hcBatt, 1.5, "h");
  }
}

void updateConnected() {
  isConnectedDisp = isConnected;
  if (!isConnected) {
    cursor(0, 0);
    Serial3.print("---No Connection----");
    tpState = tpMode = tpValSet = tpFps = tpPitch = NO_DISP;
    sonarDistance = tpBatt = tpHeadingDisp = NO_DISP;
  }
}

void setState() {
  char* stateStr;
  tpStateDisp = tpState;
  cursor(0, 0);
  if (tpState == NO_DISP) return;
  if (isStateBitSet(TP_STATE_ROUTE)) {
    stateStr = "Route";
  }
  else if (isStateBitSet(TP_STATE_RUNNING)) {
    stateStr = "Run! ";
  }
  else if (isStateBitSet(TP_STATE_RUN_READY)) {
    if (isStateBitClear(TP_STATE_UPRIGHT)) stateStr = "Fall ";
    else stateStr = "Lift ";
  }
  else {
    stateStr = "Idle ";
  }  
  Serial3.print(stateStr);
}

void setMode() {
  char* modeStr = "";
  tpModeDisp = tpMode;
  cursor(5, 0);
  switch (tpMode) {
    case NO_DISP:
      return;
      break;
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
  Serial3.print(modeStr);
}



void setValSet() {
  char* valSetStr = "";
  tpValSetDisp = tpValSet;
  cursor(9, 0);
  switch (tpValSet) {
    case NO_DISP:
      return;
      break;
    case 0:
      valSetStr = "SetA   ";
      break;
    case 1:
      valSetStr = "SetB   ";
      break;
    case 2:
      valSetStr = "SetC   ";
      break;
    default:
      valSetStr = "Set?   ";
      break;
  }
  Serial3.print(valSetStr);
}

//
//  tpFpsDisp = tpFps;
//  setFloatVal(0, 1, 3, tpFps, "fps ");
//
//void setPitch() {
//  char fill[] = {223, 32, 0};
//  tpPitchDisp = tpPitch;
//  setFloatVal(8, 1, 4, tpPitch, fill); //º
//}
//


// width is width of place for number not including the trail
void setIntVal(int x, int y, int width, int val, char* trail) {
  char spaces[] = "              ";
  int spaceCount;
  cursor(x, y);
  if (val == NO_DISP) {
    Serial3.write(spaces, width);
    Serial3.write(spaces, strlen(trail));
  }
  else {
    if (val >= 10000) spaceCount = 1;
    else if (val >= 1000) spaceCount = 2;
    else if (val >= 100) spaceCount = 3;
    else if (val >= 10) spaceCount = 4;
    else if (val >= 0) spaceCount = 5;
    else if (val > -10) spaceCount = 4;
    else if (val > -100) spaceCount = 3;
    else if (val > -1000) spaceCount = 2;
    else if (val > -10000) spaceCount = 1;
    else spaceCount = 0;
    spaceCount = (spaceCount - 5) + width;
    if (spaceCount < 0) spaceCount = 0;
    Serial3.write(spaces, spaceCount);
    Serial3.print(val);
    Serial3.print(trail);
  }
}

// "val" is an int X 100, width is number with. & sign but not trail.
void setFloatVal(int x, int y, int width, int val, char trail[]) {
  char spaces[] = "            ";
  if (val == NO_DISP) {
    Serial.write(spaces, width);
    Serial.write(spaces, strlen(trail));
  }
  else {
    int spaceCount;
    float fVal = ((float) val) / 100.0; 
    if (fVal >= 100.0) spaceCount = 1;        // 100.0 - 999.9
    else if (fVal >= 10.0) spaceCount = 2;    //  10.0 -  99.0
    else if (fVal >= 0.0) spaceCount = 3;     //   0.0 -   9.9
    else if (fVal >= -9.9) spaceCount = 2;    //  -0.1 -  -9.9
    else if (fVal >= -99.9) spaceCount = 1;   // -10.0 -  -99.9
    else  spaceCount = 0;                     // -100.0 - -999.9
    spaceCount = (spaceCount - 4) + width;
    cursor(x, y);
    Serial3.write(spaces, spaceCount);
    float v = ((float) val) / 100.0;
    Serial3.print(v, 1);
    Serial3.print(trail);
  }
}

void setBattPct(int y, int volt, float factor, char trail[]) {
  int newVolt = (int) (((float) volt) * factor);
  float pct = getPct(newVolt);
  setIntVal(16, y, 2, (int) pct, trail);
}

int getHcBatt() {
  int a = analogRead(PIN_BATT);
  hcBatt = (int) ((1000L * a) / 1120L);
  return hcBatt;
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


