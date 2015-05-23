#include "Common.h"

#define ULONG_MAX 4294967295L
#define NO_DISP 1000000

const int PIN_SWA = 2;   // Ye Shift key, lower left
const int PIN_SWB = 3;   // Gy unused (intermittent!)
const int PIN_SW_SHIFT = 4;   // Wh Debug
const int PIN_PWR = 11;
const int PIN_LED_BD = 13;

const int PIN_X = A0;     // Gn Joystick
const int PIN_Y = A1;     // Bu Joystick
const int PIN_BATT = A2;  // Battery voltage
const int PIN_VKEY = A3;  // Vi Vkey switches

unsigned long tStart = 0;

boolean lightState = false;
boolean routeState = false;
int key;
int oldKey = 99;
int oldShift = 99;

double joyX = 0.0;
double joyY = 0.0;

unsigned long activeTime = 0L;
unsigned long timeMilliseconds = 0L;
unsigned long updateTrigger = 0L;
unsigned long ledTrigger = 0L;
unsigned long timeLowBatt = ULONG_MAX;
unsigned long msgTime = 0UL;

// State bits;
boolean isDumping = false;
boolean isRouteInProgress = false;
boolean isPcActive = false;
boolean isHcActive = false;
boolean isOnGround = false;
boolean isUpright = false;
boolean isRunReady = false;
boolean isRunning = false;
boolean isHoldFps = false;
boolean isHoldHeading = false;

boolean isLightOn = false;

// Values to be displayed
boolean isConnected = true; // for proper startup
float tpPitch = 0.0;
float tpHeading = 0;
float tpFps = 0.0;
int tpState = 0;
int stateInt = 0;
int hcBatt = 0;
float tpBatt = 0;
int tpBattPct = 0;
int hcBattPct = 0;
int debug1 = 0; // Write to this to display value on row 4
int debug2 = 0; // Write to this to display value on row 4
int yaw = 12340;
float tpSonarDistance = 0;
int tpMode = MODE_TP5;
int tpValSet = 99;
int tpBattVolt = 0;
int tpRouteStep = 0;
String message = "";


/*****************************************************
 *
 *    setup() 
 *
 ****************************************************/
void setup() {
  Serial1.begin(57600); // XBee
  Serial.begin(115200); // debug

  pinMode(PIN_SWA, INPUT_PULLUP);
  pinMode(PIN_SWB, INPUT_PULLUP);
  pinMode(PIN_SW_SHIFT, INPUT_PULLUP);
  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_LED_BD, OUTPUT);
  digitalWrite(PIN_LED_BD, HIGH);
  digitalWrite(PIN_PWR, HIGH);

  lcdInit();
}

/*****************************************************
 *
 *    loop() 
 *    Whenever a SEND_STATE message is received, updateAll is called,
 *    otherwise it is called 10x per sec.
 ****************************************************/
void loop() {
  timeMilliseconds = millis();
  readXBee();
  if (timeMilliseconds > updateTrigger) { 
    updateAll();
  }
}

void updateAll() {
  updateTrigger = timeMilliseconds + 100;
  checkJoystick();
  sendJoyXY();
  checkSwitches();
  checkConnected();
  interpretState();
  lcdUpdate();
}


/*****************************************************
 *
 *    checkJoystick() 
 *
 ****************************************************/
void checkJoystick() {
   const int JOY_ZERO = 15; // Range to treat as zero.
  int a = analogRead(PIN_X);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  joyX = ((float) (a -512)) / 512.0; // scale to +- 1.0
  int b = analogRead(PIN_Y);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  joyY = ((float) (b -512)) / 512.0; // scale to +- 1.0
  if ((a != 512) || (b != 512)) {
    activeTime = timeMilliseconds;
  }
}

/*****************************************************
 *
 * checkSwitches()
 ****************************************************/
void checkSwitches() {
  int x = 0;
  int shift = digitalRead(PIN_SW_SHIFT);
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
  if (shift == HIGH) {  // Shift key not pressed
    switch (key) {
      case 1: // top row right
        sendXMsg(RCV_ROTATE, 90);
        break;
      case 2: // top row middle
        sendXMsg(RCV_ROTATE, -90);
        break;
      case 3: // top row left
        sendXMsg(RCV_RUN, (isRunReady) ? 0 : 1);
       break;
      case 4: // 2nd row right
        sendXMsg(RCV_ROUTE_ES, 0); // Route, end stand.
        break;
      case 5: // 2nd row middle
        sendXMsg(RCV_ROUTE, (isRouteInProgress) ? 0 : 1);
        break;
      case 6: // 2nd row left
        break;
      case 7: // 3rd row right
        break;
      case 8: // 3rd row middle
        break;
      case 9: // 3rd row right
        break;
      case 10: // 4th row right
        isLightOn = !isLightOn;
        sendXMsg(RCV_LIGHTS, (isLightOn) ? 1 : 0); 
        break;
      case 11: // 4th row middle
        sendXMsg(RCV_DUMP_START, 0); // Dump data
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
        sendXMsg(RCV_ROTATE, -178);
        break;
      case 2:
        sendXMsg(RCV_ROTATE, 178);
        break;
      case 3:
        break;
      case 4:
        break;
      case 5:
        break;
      case 6:
        powerDown();
        break;
      case 7:
        sendXMsg(RCV_RESET_NAV, 0);
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

int readKey() {
  int vk = analogRead(PIN_VKEY);
  return(13 - ((vk + 30) / 61));
}

void powerDown() {
  digitalWrite(PIN_PWR, LOW);
  Serial.println("PowerDown");
}



void ledBlink() {
  if (ledTrigger < timeMilliseconds) { // once/second
    ledTrigger = timeMilliseconds + 500;
    digitalWrite(PIN_LED_BD, !digitalRead(PIN_LED_BD));
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


// better to do by parsing string? or division & modulus?
void interpretState() {
  stateInt          = tpState;
  isHoldFps         = stateBit(512);
  isHoldHeading     = stateBit(256);
  isDumping         = stateBit(128);
  isRouteInProgress = stateBit(64);
  isPcActive        = stateBit(32);
  isHcActive        = stateBit(16);
  isOnGround        = stateBit(8);
  isUpright         = stateBit(4);
  isRunReady        = stateBit(2);
  isRunning         = stateBit(1);
}

boolean stateBit(int factor) {
  if (stateInt >= factor) {
    stateInt -= factor;
    return true;
  }
  return false;
}
