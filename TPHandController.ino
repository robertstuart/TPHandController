#include "Common.h"
#include <SoftwareSerial.h>

SoftwareSerial lcdSerial(12, 2); // RX, TX

#define ULONG_MAX 4294967295L
#define INT_MIN -32768

const int PIN_SW3 = 3;   // Wh Shift key, lower left
const int PIN_SW4 = 4;   // Gy unused
const int PIN_SW5 = 5;   // Wh Debug
const int PIN_PWR = 11;
const int PIN_LED = 13;

const int PIN_X = A0;     // Gn Joystick
const int PIN_Y = A1;     // Bu Joystick
const int PIN_BATT = A2;  // Battery voltage
const int PIN_VKEY = A3;  // Vi Vkey switches

unsigned long tStart = 0;

boolean lightState = false;
int key;
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
int tpBatt = 0;
int tpBattPct = 0;
int hcBattPct = 0;
int debug1 = 0; // Write to this to display value on row 4
int debug2 = 0; // Write to this to display value on row 4
int yaw = 12340;
int sonarDistance = 5670;
int tpMode = MODE_TP5;
int tpValSet = 99;
int tpBattVolt = 0;
int tpMsgRcvType = 0;
int tpMsgRcvVal = 0;


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
    if (isSerialDebug()) serialDebugOut();
  }


  checkJoystick();
  checkSwitches();
  checkConnected();
  if (ledBlink()) {
  }
  if ((timeMilliseconds - activeTime) > 300000) { // 5 minutes idle?
    powerDown();
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
  if ((a != 512) || (b != 512)) {
    activeTime = timeMilliseconds;
  }
}

/*****************************************************
 *
 * checkSwitches()
 ****************************************************/
void checkSwitches() {
  int shift = digitalRead(PIN_SW3);
  key = readKey();
  if ((key == oldKey) && (shift == oldShift)) {
    return;
  }
  else {
    for (int i = 0; i < 10; i++) {
      delayMicroseconds(100);
      if (key != readKey()) return;
    }
    oldKey = key;
    oldShift = shift;
  }
  if (key != 13) {
    activeTime = timeMilliseconds;
  }
debug1 = key;
  if (shift == HIGH) {  // Shift key not pressed
    switch (key) {
      case 1: // top row right
        break;
      case 2: // top row middle
        break;
      case 3: // top row left
        if (isStateBitSet(TP_STATE_RUN_READY))  sendMsg(TP_RCV_MSG_RUN_READY, 0);
        else sendMsg(TP_RCV_MSG_RUN_READY, 1); 
        break;
      case 4: // 2nd row right
        break;
      case 5: // 2nd row middle
        break;
      case 6: // 2nd row left
        if (isStateBitSet(TP_STATE_LIFTSENSE))  sendMsg(TP_RCV_MSG_LIFTSENSE, 0);
        else sendMsg(TP_RCV_MSG_LIFTSENSE, 1); 
        break;
      case 7: // 3rd row right
        sendMsg(TP_RCV_MSG_JUMP,0);
        break;
      case 8: // 3rd row middle
        break;
      case 9: // 3rd row right
        break;
      case 10: // 4th row right
        if (lightState) sendMsg(TP_RCV_MSG_LIGHTS, 7); // All lights on
        else sendMsg(TP_RCV_MSG_LIGHTS, 0); // All lights off
        lightState = ! lightState;
        break;
      case 11: // 4th row middle
        sendMsg(TP_RCV_MSG_DSTART,0); // Dump data
        break;
      case 12: // 4th row left
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
        sendMsg(TP_RCV_MSG_MODE, MODE_TP5);
        break;
      case 10:
        break;
      case 11:
        break;
      case 12:
        sendMsg(TP_RCV_MSG_MODE, MODE_TP6);
        break;
      default:   
        break;   
    } 
  }
}

int readKey() {
  int vk = analogRead(PIN_VKEY);
  return(13 - ((vk + 30) / 61));
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
  if (digitalRead(PIN_SW5) == LOW) return true;
  else return false;
}

void serialDebugOut() {
  // Place any debug printing here
  Serial.println(key);
}
