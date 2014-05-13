#include "Common.h"
#include <SoftwareSerial.h>

SoftwareSerial lcdSerial(12, 2); // RX, TX

#define ULONG_MAX 4294967295L

// Switch states
const int SW_SELECTED = 0;
const int SW_CLICKED = 1; // Use <= SW_CLICKED to get selected state
const int SW_UNCLICKED = 2; // Use >= SW_UNCLICKED to get selected state
const int SW_UNSELECTED = 3; 
const int SW_UNSHIFTED = 0;
const int SW_CTRL = 1;  // left lower switch
const int SW_ALT = 2;   // right lower switch
const int SW_ALT_CTRL = 3; // both 


const int PIN_SW1L = 5;
const int PIN_SW1R = 9;
const int PIN_SW2L = 3;
const int PIN_SW2R = 8;
const int PIN_SW3L = 4;
const int PIN_SW3R = 10;
const int PIN_SW4L = 7;
const int PIN_SW4R = 6;
const int PIN_PWR = 11;
const int PIN_LED = 13;

const int PIN_X = A1;
const int PIN_Y = A0;
const int PIN_BATT = A2;

unsigned long tStart = 0;

int cmdState = CMD_STATE_PWR;
int x = 0;
int y = 0;

int switchStates[] = {
  LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
int oldSwitchStates[] = {
  LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

unsigned long activeTime = 0L;
unsigned long timeMilliseconds = 0L;
unsigned long lcdUpdateTime = 0L;
unsigned long ledUpdateTime = 0L;
unsigned long lcdUpdateTrigger = ULONG_MAX;

unsigned long msgTime = 0UL;

// Values to be displayed
boolean isConnected = true; // for proper startup
int tpState = 0;
int tpStateDisp = 99;
int tpMode = MODE_TP4;
int tpModeDisp = 99;
int tpValSet = 0;
int tpValSetDisp = 99;
int tpBattVolt = 0;
unsigned long localBatteryUpdateTime = 0L;
unsigned long tpBatteryUpdateTime = 0L;

// Status values on bottom line.  minus values are not displayed.
int row4Values[] = {
  -1,-1,-1,-1,-1};
int row4ValuesDisplay[] = {
  -1,-1,-1,-1,-1};

/*****************************************************
 *
 *    setup() 
 *
 ****************************************************/
void setup() {
  pinMode(PIN_SW1L, INPUT_PULLUP);
  pinMode(PIN_SW1R, INPUT_PULLUP);
  pinMode(PIN_SW2L, INPUT_PULLUP);
  pinMode(PIN_SW2R, INPUT_PULLUP);
  pinMode(PIN_SW3L, INPUT_PULLUP);
  pinMode(PIN_SW3R, INPUT_PULLUP);
  pinMode(PIN_SW4L, INPUT_PULLUP);
  pinMode(PIN_SW4R, INPUT_PULLUP);
  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_PWR, HIGH);

  lcdInit();
  Serial.begin(57600); //FDDI and XBee
}

/*****************************************************
 *
 *    loop() 
 *
 ****************************************************/
void loop() {
  timeMilliseconds = millis();
  if (readXBee()) { // true if we have just finished sending packet.
    if ((tpState & TP_STATE_STREAMING) == 0) {
      lcdUpdateTrigger = timeMilliseconds + 5; // Wait until write is complete.
    }
  }
  if (!isConnected) {
    if (lcdUpdateTrigger == ULONG_MAX) {
      lcdUpdateTrigger = timeMilliseconds + 100;
    }
  }
  if (timeMilliseconds > lcdUpdateTrigger) {
    if ((tpState & TP_STATE_STREAMING) == 0) {
      lcdUpdate();
    }
    lcdUpdateTrigger = ULONG_MAX;      
  }
  checkJoystick();
  checkSwitches();
  ledBlink();
  checkConnected();
}

/*****************************************************
 *
 *    checkJoystick() 
 *
 ****************************************************/
void checkJoystick() {
  int a = analogRead(PIN_X);
  x = (a -512) / 4; // scale to +- 128
  int b = analogRead(PIN_Y);
  y = -((b -512) / 4); // scale to +- 128
}

/*****************************************************
 *
 * checkSwitches()
 *               UNSHIFTED       CTRL           ALT       ALT_CTRL
 *-------
 * PIN_SW1L -  90deg Left        Run
 * PIN_SW1R -  90deg Right       Halt
 *-------
 * PIN_SW2L -  stream         
 * PIN_SW2R -  non-stream          
 *-------
 * PIN_SW3L -  running                                  Power down HC
 * PIN_SW3R -  idle                                     Power down TP
 *-------
 * PIN_SW4L ----------  CTRL shift key --------------        
 * PIN_SW4R ----------   ALT shift key --------------    
 *
 *Need to debounce?
 * Note: Hold down PIN_SW1L when booting to change LCD
 ****************************************************/
void checkSwitches() {
  int shift = shiftState();
  //      lcdStatusLine(shift,1);
  switch(shift) {
  case SW_UNSHIFTED:
    if (switchState(PIN_SW1L) == SW_CLICKED) {
    }
    if (switchState(PIN_SW1R) == SW_CLICKED) {
    }
    if (switchState(PIN_SW2L) == SW_CLICKED) {
    }
    if (switchState(PIN_SW2R) == SW_CLICKED) {
    }
    if (switchState(PIN_SW3L) == SW_CLICKED) {
    }
    if (switchState(PIN_SW3R) == SW_CLICKED) {
    }
    break;

  case SW_CTRL:
    if (switchState(PIN_SW1L) == SW_CLICKED) {
      cmdState = cmdState | CMD_STATE_RUN;
    }
    if (switchState(PIN_SW1R) == SW_CLICKED) {
      cmdState = cmdState & (~CMD_STATE_RUN);
    }
    if (switchState(PIN_SW2L) == SW_CLICKED) {
      cmdState = cmdState | CMD_STATE_STREAM;
    }
    if (switchState(PIN_SW2R) == SW_CLICKED) {
      cmdState = cmdState & (~CMD_STATE_STREAM);
    }
    break;

  case SW_ALT:
    break;

  case SW_ALT_CTRL:
    if (switchState(PIN_SW3L) == SW_CLICKED) {
      powerDown();
    }
    if (switchState(PIN_SW3R) == SW_CLICKED) {
      cmdState = cmdState & (~CMD_STATE_PWR);
    }
    break;
  }  // end switch(shiftState()

}

void powerDown() {
  digitalWrite(PIN_PWR, LOW);
}

/*****************************************************
 *
 *    switchState() 
 *
 ****************************************************/
int switchState(int switchPin) {
  int sState = digitalRead(switchPin);
  if (sState != switchStates[switchPin]) {
    switchStates[switchPin] = sState;
    oldSwitchStates[switchPin] = sState;
    if (sState == LOW) {
      return SW_CLICKED;
    }
    else {
      return SW_UNCLICKED;
    }
  }
  if (sState == LOW) {
    return SW_SELECTED;
  }
  return SW_UNSELECTED;
}

int shiftState() {
  int ctrl = switchState(PIN_SW4L);
  int alt = switchState(PIN_SW4R);
  if ((ctrl <= SW_CLICKED) && (alt <= SW_CLICKED)) {
    return SW_ALT_CTRL;
  }
  else if (ctrl <= SW_CLICKED) {
    return SW_CTRL;
  }
  else if (alt <= SW_CLICKED) {
    return SW_ALT;
  }
  return SW_UNSHIFTED;
}

void ledBlink() {
  if ((ledUpdateTime + 500) < timeMilliseconds) { // once/second
    ledUpdateTime = timeMilliseconds;
    if (digitalRead(PIN_LED) == HIGH) {
      digitalWrite(PIN_LED, LOW);
    }
    else {
      digitalWrite(PIN_LED, HIGH);
    }
  }
} // end ledBlink()

void checkConnected() {
  if ((msgTime + 500) > timeMilliseconds) {
    isConnected = true;
  }
  else {
    isConnected = false;
  }
}

