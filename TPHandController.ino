#include "Common.h"


const int PIN_JOY_SWITCH = 8  ;
const int PIN_SW1 = 7;
const int PIN_SW2 = 6;
const int PIN_SW3 = 5;
const int PIN_SW4 = 4;
const int PIN_SW5 = 3;
const int PIN_SW6 = 2;

const int PIN_BATT = A0;
const int PIN_X = A2;
const int PIN_Y = A1;

unsigned long trigger = 0L;
unsigned long ledTrigger = 0L;
float yVal;
float xVal;
int joySw = HIGH;
int sw1 = HIGH;
int sw2 = HIGH;
int sw3 = HIGH;
int sw4 = HIGH;
int sw5 = HIGH;
int sw6 = HIGH;
float tpBatteryVolt = 0.0f;
boolean isConnected = false;
int runState = STATE_UNKNOWN;
int valSetStat = VAL_SET_A;
int mode = MODE_TP4;
int runSubState;
int errorCount = 0;
unsigned long dsTrigger;
unsigned long secTrigger;
unsigned long timeMilliseconds = 0L;
unsigned int unknownSingleCmd = 0;
unsigned int unknownParamCmd = 0;
unsigned int byteCountErrors = 0;
unsigned int pingHcCount = 0; // Ping number sent to tp
unsigned int pingTpErrors = 0;
unsigned long pingTpTime = 0;
int pingErrorsTpHc = 0;

void setup() {
  lcdInit();
  Serial.begin(115200);
  Serial1.begin(57600);
  pinMode(PIN_LED, OUTPUT);   
  pinMode(PIN_JOY_SWITCH, INPUT_PULLUP);
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  pinMode(PIN_SW3, INPUT_PULLUP);
  pinMode(PIN_SW4, INPUT_PULLUP);
  pinMode(PIN_SW5, INPUT_PULLUP);
  pinMode(PIN_SW6, INPUT_PULLUP);
  secTrigger = dsTrigger = millis();
}

void loop() {
  readTp();  // Read any commands from tp

    timeMilliseconds = millis();

  if (timeMilliseconds > dsTrigger) {
    dsTrigger += 100;  // 10/sec
    checkJoystick();
    checkSwitches();
    ping();
    lcdUpdate();
  } // end dsTrigger
}

void checkJoystick() {
  int a = analogRead(PIN_X);
  int x = (a -512) / 8; // scale to +- 64
  cmdOne(CMD_X, x);
  int b = analogRead(PIN_Y);
  int y = -((b -512) / 8); // scale to +- 64
  cmdOne(CMD_Y, y);
}

/*****************************************************
 *
 *                                  ModeA             ModeB
 * swJoy - pressing on joystick
 * sw1   - top row outside          90deg Left  
 * sw2   - top row inside           90deg Right  
 * sw3   - second row
 * sw4   - third row outside
 * sw5   - third row inside                           change TP mode
 * sw6   - fourth row               Mode A & B
 *
 ****************************************************/

void checkSwitches() {
  sw6 = digitalRead(PIN_SW6) == LOW;  // switch mode

  // Joystick switch,
  int sw = digitalRead(PIN_JOY_SWITCH) == LOW;
  if (joySw != sw) {
    int newState = STATE_READY;
    joySw = sw;
    if (joySw) { // state changed to pressed
      // toggle the state
      if (runState != STATE_RESTING) {
        newState = STATE_RESTING;
      }
      //      lcdStatus();
      cmdOneU(CMD_RUN_STATE, newState);
    }  
  }

  // Switch 1, A = rotate left 90, B = 180
  sw = digitalRead(PIN_SW1) == LOW;
  if (sw1 != sw) {
    sw1 = sw;
    if (sw1) { // state changed to press
      if (sw6) {
        cmdOneU(CMD_VAL_SET, VAL_SET_A);
      }
      else {
        cmdTwo(CMD_ROTATE, 90);
      }
    }  
  }

  // Switch 2, A = rotate right 90
  sw = digitalRead(PIN_SW2) == LOW;
  if (sw2 != sw) {
    sw2 = sw;
    if (sw2) {  // changed to pressed
      if (sw6) {
        cmdOneU(CMD_VAL_SET, VAL_SET_B);
      }
      else {
        cmdTwo(CMD_ROTATE, -90);
      }
    }  
  }

  // Switch 3, 
  sw = digitalRead(PIN_SW3) == LOW;
  if (sw3 != sw) {
    sw3 = sw;
    if (sw3) {  // changed to pressed
      if (sw6) {
        cmdOneU(CMD_VAL_SET, VAL_SET_C);
      }
      else {
        
      }
    }  
  }

  // Switch 4, Performance level
  sw = digitalRead(PIN_SW4) == LOW;
  if (sw2 != sw) {
    sw2 = sw;
    if (sw2) {  // changed to pressed
      //      cmdInt(CMD_PERFORMANCE, 0);
    }  
  }

  // Switch 5, A = , B = mode change
  sw = digitalRead(PIN_SW5) == LOW;
  if (sw5 != sw) {
    sw5 = sw;
    if (sw5) {  // pressed?
      if (sw6) {
        changeMode();
      }
    }  
  }

}


// Send a ping and check to see if we are receiving pings
void ping() {
  pingHcCount = ++pingHcCount % 128;
  cmdOneU(CMD_TO_TP_PING, pingHcCount);
  if ((timeMilliseconds - pingTpTime) > 500) {
    isConnected = false;
  } 
  else {
    isConnected = true;
  }
}

void changeMode() {
  int newMode = MODE_PID1;
  switch (mode) {
  case MODE_PID1:
    newMode = MODE_TP4;
    break;
  case MODE_TP4:
    newMode = MODE_TP3;
    break;
  default:
    break;
  }
  //  cmdInt(CMD_MODE, newMode);
}



