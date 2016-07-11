#include "Common.h"

#define XBEE_SER Serial1

#define ULONG_MAX 4294967295L
#define NO_DISP 1000000

const int PIN_SWA = 2;   // Ye Shift key, lower left
const int PIN_SWB = 3;   // Gy unused (intermittent!)
const int PIN_SW_SHIFT = 4;   // Wh Debug
const int PIN_PWR = 11;
const int PIN_LED_BD = 13;

const int PIN_X_A = A0;     // Gn Joystick A
const int PIN_Y_A = A1;     // Bu Joystick A
const int PIN_BATT = A2;  // Battery voltage
const int PIN_VKEY = A3;  // Vi Vkey switches
const int PIN_Y_B = A4;  // Joystick B
const int PIN_X_B = A5;  // Joystick B

unsigned long tStart = 0;

boolean lightState = false;
boolean routeState = false;
int key;
int oldKey = 99;
int oldShift = 99;

double joyAX = 0.0;
double joyAY = 0.0;
double joyBX = 0.0;
double joyBY = 0.0;

unsigned long timeMilliseconds = 0L;
unsigned long updateTrigger = 0L;
unsigned long ledTrigger = 0L;
unsigned long timeLowBatt = ULONG_MAX;
unsigned long msg2Time = 0UL;
unsigned long msg3Time = 0UL;

// State bits;
boolean is2Upright = false;
boolean is2RunReady = false;
boolean is2Running = false;
boolean is2RouteInProgress = false;
boolean is3RunReady = false;
boolean is3RouteInProgress = false;

boolean isLightOn = false;

// Values to be displayed
boolean is2Connected = true; // for proper startup
boolean is3Connected = true; // for proper startup
float p2Pitch = 0.0;
float p3Pitch = 0.0;
float p2Heading = 0.0;
float p3Heading = 0.0;
float p2Fps = 0.0;
float p3Fps = 0.0;
int p2State = 0;
int p3State = 0;
int stateInt = 0;
float hcBatt = 0;
float p2Batt = 0;
float p3ABatt = 0; // Low voltage battery
float p3BBatt = 0; // High voltage battery
int hcBattPct = 0;
int p2BattPct = 0;
int p3ABattPct = 0;
int p2BBattPct = 0;
float p2SonarDistanceR = 0.0;
float p2SonarDistanceL = 0.0;
int tpMode = MODE_TP5;
int tpValSet = 99;
int tpBattVolt = 0;
int tpRouteStep = 0;
String p2Message = "";
String p3Message = "";


/*****************************************************
 *
 *    setup() 
 *
 ****************************************************/
void setup() {
  XBEE_SER.begin(57600); // XBee
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
  static unsigned int subCycle = 0;
  timeMilliseconds = millis();
  readXBee();
  checkSwitches();
  if (timeMilliseconds > updateTrigger) { 
    updateTrigger = timeMilliseconds + 30;
    subCycle = ++subCycle % 3;
    if (subCycle == 0) { // Transmit to TwoPotatoe
        checkAJoystick();
        send2Potatoe();
    } else if (subCycle == 1) { // Transmit to ThreePotoate
        checkBJoystick();
        send3Potatoe();
    } else { // Do misc tasks while 2&3Potatoe communicate
      lcdUpdate();
      checkConnected();
    }
  }
}



/*****************************************************
 *
 *    checkJoystick() 
 *
 ****************************************************/
void checkAJoystick() {
  const int JOY_ZERO = 15; // Range to treat as zero.

  // joystic A
  int a = analogRead(PIN_X_A);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  joyAX = ((float) (a -512)) / 512.0; // scale to +- 1.0
  int b = analogRead(PIN_Y_A);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  joyAY = ((float) (b -512)) / 512.0; // scale to +- 1.0
}

void checkBJoystick() {
  const int JOY_ZERO = 15; // Range to treat as zero.
  
  // joystic B
  int a = 1024 - analogRead(PIN_X_B);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  joyBX = ((float) (a -512)) / 512.0; // scale to +- 1.0
  int b = 1024 - analogRead(PIN_Y_B);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  joyBY = ((float) (b -512)) / 512.0; // scale to +- 1.0
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
  if (shift == HIGH) {  // Shift key not pressed
    switch (key) {
      case 1: // top row right
        isLightOn = !isLightOn;
        send2Msg(RCV_LIGHTS, (isLightOn) ? 1 : 0); 
        break;
      case 2: // top row middle
        if (is2RouteInProgress) send2Msg(RCV_ROUTE, 0);
        else send2Msg(RCV_ROUTE, 1);
        break;
      case 3: // top row left
        send2Msg(RCV_RUN, (is2RunReady) ? 0 : 1);
       break;
      case 4: // 2nd row right
        send2Msg(RCV_DUMP_START, 0); // Dump data
        break;
      case 5: // 2nd row middle
        send2Msg(RCV_ROUTE_ES, 0);
        break;
      case 6: // 2nd row left
        break;
      case 7: // 3rd row right
        break;
      case 8: // 3rd row middle
        if (is3RouteInProgress) send3Msg(RCV_ROUTE, 0);
        else send3Msg(RCV_ROUTE, 1);
         break;
      case 9: // 3rd row Left,  
        send3Msg(RCV_RUN, (is3RunReady) ? 0 : 1);
        break;
      case 10: // 4th row right
        send3Msg(RCV_DUMP_START, 0); // Dump data
        break;
      case 11: // 4th row middle
        send3Msg(RCV_ROUTE_ES, 0);
        break;
      case 12: // 4th row left
         break;
      default: 
        break;     
    } 
  } 
  else {  // Shift key pressed.
    switch (key) {
      case 1:  // top row right
        break;
      case 2: // top row middle
        send2Msg(RCV_SET_ROUTE, 1);
        break;
      case 3: // top row left
        break;
      case 4: // 2nd row right
        break;
      case 5: // 2nd row middle 
        send2Msg(RCV_SET_ROUTE, 0);
        break;
      case 6: // 2nd row left
        break;
      case 7: // 3rd row right
        break;
      case 8: // 3rd row middle
        send3Msg(RCV_SET_ROUTE, 1);
        break;
      case 9: // 3rd row Left
        break;
      case 10: // 4th row right
        powerDown();
        break;
      case 11: // 4th row middle
        send3Msg(RCV_SET_ROUTE, 0);
        break;
      case 12: // 4th row left
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
  if ((msg2Time + 500) > timeMilliseconds) {
    is2Connected = true;
  }
  else {
    is2Connected = false;
  }
  if ((msg3Time + 500) > timeMilliseconds) {
    is3Connected = true;
  }
  else {
    is3Connected = false;
  }
}


void interpret2State(int state) {
  p2State = state;
  is2Running = (p2State & 1) > 0;
  is2RunReady = (p2State & 2) > 0;
  is2Upright = (p2State & 4) > 0;
  is2RouteInProgress = (p2State & 64) > 0;
}
void interpret3State(int state) {
  p3State = state;
  is3RunReady = (p3State & 2) > 0;
  is3RouteInProgress = (p3State & 64) > 0;
}

