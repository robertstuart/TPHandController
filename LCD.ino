unsigned int updateRow = 0;
float hcBattDisp = 0;
float p2BattDisp = 0;
float p3ABattDisp = 0;
float p3BBattDisp = 0;
float sonarDistanceRDisp = 0;
float sonarDistanceLDisp = 0;
boolean is2ConnectedDisp = true;
boolean is3ConnectedDisp = true;
int p2StateDisp = 0;
float p3StateDisp = 0;
float p2HeadingDisp = 0;
float p2FpsDisp = 0.0;
float p3FpsDisp = 0.0;

unsigned int p2MessageTrigger = 0;
unsigned int p3MessageTrigger = 0;

String p2MessageDisp = "";
String p3MessageDisp = "";
String spacex = String("                    ");
char numBuf[] = "           ";

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
 *    lcdUpdate()
 ****************************************************/
void lcdUpdate() {
  static int loop = 0;
  loop = ++loop % 10;

  // Connections & State
  if (is2ConnectedDisp != is2Connected) {
    is2ConnectedDisp = is2Connected;
    if (!is2Connected) {
      cursor(0, 0);
      Serial3.print("---No Connection----");
    }
  }
  if (p2StateDisp != p2State) {
    set2State();
  }
  if (is3ConnectedDisp != is3Connected) {
    is3ConnectedDisp = is3Connected;
    if (!is3Connected) {
      cursor(0, 2);
      Serial3.print("---No Connection----");
    }
  }
  if (p3StateDisp != p3State) {
    set3State();
  }

  // Speed
  if (p2FpsDisp != p2Fps) {
    if (timeMilliseconds > p2MessageTrigger) {
      p2FpsDisp = p2Fps;
      snprintf(numBuf, 6, "%5.1f", p2Fps);
      cursor(0, 1);
      Serial3.print(numBuf);
      Serial3.print("fps  ");
    }
  }
  if (p3FpsDisp != p3Fps) {
    if (timeMilliseconds > p3MessageTrigger) {
      p3FpsDisp = p3Fps;
      snprintf(numBuf, 6, "%5.1f", p3Fps);
      cursor(0, 3);
      Serial3.print(numBuf);
      Serial3.print("fps  ");
    }
  }

  // Sonar
  if (sonarDistanceLDisp != p2SonarDistanceL) {
    sonarDistanceLDisp = p2SonarDistanceL;
    snprintf(numBuf, 5, "%4.1f", p2SonarDistanceL);
    cursor(6, 0);
    Serial3.print(numBuf); 
    Serial3.print("  ");
  }
  if (sonarDistanceRDisp != p2SonarDistanceR) {
    sonarDistanceRDisp = p2SonarDistanceR;
    snprintf(numBuf, 5, "%4.1f", p2SonarDistanceR);
    cursor(11, 0);
    Serial3.print(numBuf); 
    Serial3.print("  ");
  }

  // Messages
  if (!p2MessageDisp.equals(p2Message)) {
    p2MessageDisp = p2Message;
    String tmpStr = String(p2Message);
    tmpStr.concat(spacex);
    tmpStr = tmpStr.substring(0,20);
    cursor(0,1);
    Serial3.print(tmpStr);
    p2MessageTrigger = timeMilliseconds + 10000;
  }
  if (!p3MessageDisp.equals(p3Message)) {
    p3MessageDisp = p3Message;
    String tmpStr = String(p3Message);
    tmpStr.concat(spacex);
    tmpStr = tmpStr.substring(0,20);
    cursor(0,3);
    Serial3.print(tmpStr);
    p3MessageTrigger = timeMilliseconds + 10000;
  }

  // Heading
  if (p2HeadingDisp != p2Heading) {
    static char fill[] = {223, 0};
    if (timeMilliseconds > p2MessageTrigger) {
      p2HeadingDisp = p2Heading;
      snprintf(numBuf, 5, "%4d", (int) p2Heading);
      cursor(11, 1);
      Serial3.print(numBuf);
      Serial3.print(fill);
    }
  }

  // Batteries
  if ((loop == 5) && (hcBattDisp != getHcBatt())) {
    hcBattDisp = hcBatt;
    setBattPct(0, hcBatt, 2, "h");
  }
  if (p2BattDisp != p2Batt) {
    p2BattDisp = p2Batt;
    setBattPct(1, p2Batt, 5, "b"); // 5 cells
  }
  if (p3ABattDisp != p3ABatt) {
    p3ABattDisp = p3ABatt;
    setBattPct(2, p3ABatt, 2, "a");
  }
  if (p3BBattDisp != p3BBatt) {
    p3BBattDisp = p3BBatt;
    setBattPct(3, p3BBatt, 10, "b");
  }
}


void set2State() {
  char* stateStr;
  p2StateDisp = p2State;
  cursor(0, 0);
  if (is2RouteInProgress) {
    stateStr = "Route";
  } else if (is2Running) {
    stateStr = "Run! ";
  } else if (is2RunReady) {
    if (is2Upright) stateStr = "Lift ";
    else stateStr = "Fall ";
  } else {
    stateStr = "Idle ";
  }  
  Serial3.print(stateStr);
}

void set3State() {
  char* stateStr;
  p3StateDisp = p3State;
  cursor(0, 2);
  if (is3RouteInProgress) {
    stateStr = "Route";
  } else if (is3RunReady) stateStr = "Run!             ";
  else stateStr =            "Idle             ";  
  Serial3.print(stateStr);
}


void setString(int x, int y, int width, String s, String append) {
  cursor(x,y);
  String tmpStr = String(spacex);
  tmpStr.concat(s);
  int len = tmpStr.length();
  tmpStr = tmpStr.substring(len - width, len);
  Serial3.print(tmpStr);
  Serial3.print(append);
}



void setBattPct(int y, float volt, int cells, char trail[]) {
  float newVolt =  volt / ((float) cells);
  float pct = getPct(newVolt);
  cursor(17, y);
  snprintf(numBuf, 3, "%02d", (int) pct);
  Serial3.print(numBuf);
  Serial3.print(trail);
//  setIntVal(1, y, 2, (int) pct, trail);
}

float getHcBatt() {
  float a = (float) analogRead(PIN_BATT);
  hcBatt = a * 0.00911;
Serial.println(hcBatt);
  return hcBatt;
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


