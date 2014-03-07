int update = 0;
boolean lcdIsConnectedDisp = true; // force display at init
int lcdRunStateDisp = 99;
int lcdModeDisp = 99;
int lcdValSetDisp = 99;

void lcdInit() {
  Serial2.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up
  clearScreen(); 
  lcdRunStateDisp = 99;
  runState = STATE_UNKNOWN;
  lcdModeDisp = 99;
  mode = MODE_UNKNOWN;
  lcdValSetDisp = 99;
  valSetStat = VAL_SET_UNKNOWN;
}

// Update the screen, only one line per decisecond
void lcdUpdate() {
  update = ++update % 6;
  switch (update) {
  case 0:
    lcdLocalBattery();
    break;
  case 1:
    lcdTpBattery();
    break;
  case 2:
    lcdState(); // Run State
    break;
  case 3:
    lcdMode(); // Algorithm
    break;
  case 4: 
    lcdValSet(); // Algorithm value set
    break;
  case 5: 
    lcdErrorStat(); // show errors
    break;
  default:
    break;
  }
}

void lcdTpBattery() {
  if (isConnected) {
    printVolt(10, 0, tpBatteryVolt);
    printPct(16, 0, true, tpBatteryVolt);
  }
}

// show lost connection, ready, idle, running, etc.
void lcdState() {
  if (isConnected != lcdIsConnectedDisp) {
    lcdIsConnectedDisp = isConnected;
    if (!isConnected) {
      cursor(0, 0);
      Serial2.print("-- No Connection ---");
    }
    else {
      lcdRunStateDisp = 99; // force refresh
    }
  }
//debugInt("runState: ", runState);
//debugInt("lcdRunStateDisp: ", lcdRunStateDisp);
//debugBoolean("isConnected: ", isConnected);
  
  if (isConnected && (runState != lcdRunStateDisp)) {
    lcdRunStateDisp = runState;
    cursor(0,0);
    switch (runState) {
    case STATE_RESTING:
      Serial2.print("Resting   ");
      break;
    case STATE_READY:
      Serial2.print("Ready     ");
      break;
    case STATE_RUNNING:
      Serial2.print("Running   ");
      break;
   default:
      Serial2.print("          ");
      break;      
    }
  }
}

void lcdLocalBattery() {
    int a = analogRead(PIN_BATT);
    float volt = ((float) a) * .0092;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 a * 0.0092f;
    printVolt(10, 1, volt);
    printPct(16, 1, false, volt);
}

void printVolt(int x, int y, float volt) {
    cursor(x, y);
    if (volt < 10.00) {
      Serial2.print(" ");
    }
    Serial2.print(volt);
    Serial2.print("V");
}

void printPct(int x, int y, boolean src, float volt) {
  cursor(x, y);
  int pct = (int) getPct(src, volt);
  if (pct < 10) {
      Serial2.print("  ");
  }
  else if (pct < 100) {
      Serial2.print(" ");
  } 
  Serial2.print(pct);
  Serial2.print("%");
}

void lcdMessage(char str[]) {
  cursor(0,3);
  Serial2.print(str);
}

void lcdMode() {
  if (!isConnected) {
    cursor(0, 1);
    Serial2.print("          ");
    lcdModeDisp = 99; // force refresh next time
    return;
  }

  if (lcdModeDisp != mode) {
    lcdModeDisp = mode;
    cursor(0, 1);
    switch (mode) {
    case MODE_PID1:
      Serial2.print("Mode: PID1");
      break;
    case MODE_TP4:
      Serial2.print("Mode: TP4 ");
      break;
    case MODE_TP3:
      Serial2.print("Mode: TP3 ");
      break;
    default:
      Serial2.print("Mode: ??? ");
      break;  
    }  
  }
}

void lcdValSet() {
  if (!isConnected) {
    cursor(0, 2);
    Serial2.print("          ");
    lcdValSetDisp = 99; // force refresh next time
    return;
  }
  
  if (lcdValSetDisp != valSetStat) {
    lcdValSetDisp = valSetStat;
    cursor(0,2);
    switch(valSetStat) {
    case VAL_SET_A:
      Serial2.print("Set: A     ");
      break;
    case VAL_SET_B:
      Serial2.print("Set: B     ");
      break;
    case VAL_SET_C:
      Serial2.print("Set: C     ");
      break;
    default:
      Serial2.print("           ");
      break;
    }
  }

}

float tpVolt[] = { 12.6f, 11.99f, 11.75f, 11.54f, 11.23f, 11.05f, 10.83f, 10.63f, 10.35f, 9.0f};
float tpPct[] =  {100.0f, 83.0f,  71.0f,  60.0f,  34.0f,  20.0f,  07.0f,  05.0f,  03.0f,  0.0f};
float localVolt[] = {  8.4f,  8.09f, 7.57f, 7.34f, 7.12f, 6.0f};
float localPct[] =  {100.0f, 88.0f, 49.0f,  20.0f, 5.0f,  0.0f};
	
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
  Serial2.write(0xFE);
  Serial2.write(base);
}

void clearScreen()
{
  Serial2.write(0xFE);
  Serial2.write(0x01); 
}

void lcdErrorStat() {
   lcdStatusLine(byteCountErrors, 3);
   lcdStatusLine(pingTpErrors, 4);
}



// print 3-digit integer in one of 5 positions on bottom
void lcdStatusLine(int i, int pos) {
  cursor(4 * pos, 3);
    if (i < 10) {
      Serial2.print("   ");
    } else if (i < 100) {
      Serial2.print("  ");
    } else if (i < 1000) {
      Serial2.print(" ");
    } else {
      Serial2.print(" ERR");
      return;
    }
    Serial2.print(i);
}

//
//void setSplash() {
//  cursor(0,0);
//  Serial2.print("     TwoPotatoe     ");
//  cursor(0, 1);
//  Serial2.print("   Hand Controller  ");
//  Serial2.write(124);
//  Serial2.write(10);
//}
