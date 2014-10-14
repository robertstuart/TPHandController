#include "Common.h"
#include <SoftwareSerial.h>

SoftwareSerial lcdSerial(12, 2); // RX, TX

#define ULONG_MAX 4294967295L
#define INT_MIN -32768

const int PIN_SW3 = 3;   // Shift key, lower left
const int PIN_SW4 = 4;   // Debut key, ??right
const int PIN_SW5 = 5;
const int PIN_PWR = 11;
const int PIN_LED = 13;

const int PIN_X = A0;     // Joystick
const int PIN_Y = A1;     // Joystick
const int PIN_BATT = A2;  // Battery voltage
const int PIN_VKEY = A3;  // Vkey switches

unsigned long tStart = 0;

boolean lightState = false;
int oldKey = 99;
int oldShift = 99;

int x = 0;
int y = 0;

unsigned long activeTime = 0L;
unsigned long timeMilliseconds = 0L;
unsigned long lcdUpdateTrigger = 0L;
unsigned long ledUpdateTrigger = 0L;
unsigned long timeLowBatt = ULONG_MAX;
unsigned long msgTime = 0UL;

// Values to be displayed
boolean isConnected = true; // for proper startup
float tpPitch = 0.0;
float tpFps = 0.0;
int tpState = 0;
int hcBatt = 0;
int tpBMBatt = 0;
int tpEMBatt = 0;
int tpLBatt = 0;
int tpBMBattPct = 0;
int tpEMBattPct = 0;
int tpLBattPct = 0;
int hcBattPct = 0;
int debug1 = 0;
int debug2 = 0;
int tpMode = MODE_TP4;
int tpValSet = 99;
int tpBattVolt = 0;
int tpMsgRcvType = 0;
int tpMsgRcvVal = 0;


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
  pinMode(PIN_SW3, INPUT_PULLUP);
  pinMode(PIN_SW4, INPUT_PULLUP);
  pinMode(PIN_SW5, INPUT_PULLUP);
  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_PWR, HIGH);

  lcdInit();
  Serial.begin(57600); //FTDI and XBee
}

/*****************************************************
 *
 *    loop() 
 *
 ****************************************************/
void loop() {
  timeMilliseconds = millis();
  if (readXBee() || (timeMilliseconds > lcdUpdateTrigger)) { // true if we have just finished sending packet.
    lcdUpdate();
    if (isSerialDebug) serialDebugOut();
  }


  checkJoystick();
  checkSwitches();
  checkConnected();
  if (ledBlink()) {
//    cursor(0,0);
//    int a = analogRead(PIN_VKEY);
//    lcdSerial.print(a);
//    lcdSerial.print("   ");
//    cursor(0,1);
//    int key = 13 - ((a + 30) / 61);
//    lcdSerial.print(key);
//    lcdSerial.print("   ");
//    serialDebugOut();
  }
}

/*****************************************************
 *
 *    checkJoystick() 
 *
 ****************************************************/
void checkJoystick() {
   const int JOY_ZERO = 15; // Range to treat a zero.
  int a = analogRead(PIN_X);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  x = (a -512) / 4; // scale to +- 128
  int b = analogRead(PIN_Y);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  y = ((b -512) / 4); // scale to +- 128
}

/*****************************************************
 *
 * checkSwitches()
 ****************************************************/
void checkSwitches() {
  int key = 13 - ((analogRead(PIN_VKEY) + 30) / 61);
  int shift = digitalRead(PIN_SW3);
  if ((key == oldKey) && (shift == oldShift)) {
    return;
  }
  else {
    oldKey = key;
    oldShift = shift;
  }
  if (shift == HIGH) {  // Shift key not pressed
    switch (key) {
      case 1:
        break;
      case 2:
        break;
      case 3:
        sendMsg(TP_RCV_MSG_RUN_READY,1); 
        break;
      case 4:
        break;
      case 5:
        break;
      case 6:
        sendMsg(TP_RCV_MSG_RUN_READY,0); 
        break;
      case 7:
        break;
      case 8:
        break;
      case 9:
        break;
      case 10:
        if (lightState) sendMsg(TP_RCV_MSG_LIGHTS, 7); // All lights on
        else sendMsg(TP_RCV_MSG_LIGHTS, 0); // All lights off
        lightState = ! lightState;
        break;
      case 11:
        break;
      case 12:
        break;
      default: 
        break;     
    } 
  } 
  else {  // Shift key pressed.
    switch (key) {
      case 1:
        break;
      case 2:
        break;
      case 3:
        sendMsg(TP_RCV_MSG_POWER,0);
        break;
      case 4:
        break;
      case 5:
        break;
      case 6:
        powerDown();
        break;
      case 7:
        break;
      case 8:
        break;
      case 9:
        break;
      case 10:
        break;
      case 11:
        break;
      case 12:
        break;
      default:   
        break;   
    } 
  }
}

void powerDown() {
  digitalWrite(PIN_PWR, LOW);
  Serial.println("PowerDown");
}

/*****************************************************
 *
 *    switchState() 
 *
 ****************************************************/
int switchState(int switchPin) {
//  int sState = digitalRead(switchPin);
//  if (sState != switchStates[switchPin]) {
//    switchStates[switchPin] = sState;
//    oldSwitchStates[switchPin] = sState;
//    if (sState == LOW) {
//      return SW_CLICKED;
//    }
//    else {
//      return SW_UNCLICKED;
//    }
//  }
//  if (sState == LOW) {
//    return SW_SELECTED;
//  }
//  return SW_UNSELECTED;
}

int shiftState() {
//  int ctrl = switchState(PIN_SW4L);
//  int alt = switchState(PIN_SW4R);
//  if ((ctrl <= SW_CLICKED) && (alt <= SW_CLICKED)) {
//    return SW_ALT_CTRL;
//  }
//  else if (ctrl <= SW_CLICKED) {
//    return SW_CTRL;
//  }
//  else if (alt <= SW_CLICKED) {
//    return SW_ALT;
//  }
//  return SW_UNSHIFTED;
}

boolean ledBlink() {
  if (ledUpdateTrigger < timeMilliseconds) { // once/second
    ledUpdateTrigger = timeMilliseconds + 500;
    if (digitalRead(PIN_LED) == HIGH) {
      digitalWrite(PIN_LED, LOW);
      return true;
    }
    else {
      digitalWrite(PIN_LED, HIGH);
      return true;
    }
  }
  return false;
} // end ledBlink()

void checkConnected() {
  if ((msgTime + 500) > timeMilliseconds) {
    isConnected = true;
  }
  else {
    isConnected = false;
  }
}

// Toggle the value so tp knows it is a new message.
boolean toggle() {
  static boolean tg = false;
  if (tg) tg = false;
  else tg = true;
  return tg;
}

// Check to see if the debug key is pressed.
boolean isSerialDebug() {
  if (digitalRead(PIN_SW3) == LOW) return true;
  else return false;
}

void serialDebugOut() {
    int a = analogRead(PIN_VKEY);
  int key = 13 - ((a + 22) / 40);
  Serial.print(key);
  Serial.print("\t");
  Serial.println(a);
  if (digitalRead(PIN_SW3) == LOW) Serial.println("sw3");
  if (digitalRead(PIN_SW4) == LOW) Serial.println("sw4");
  if (digitalRead(PIN_SW5) == LOW) Serial.println("sw5");
}
