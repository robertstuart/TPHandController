#include "Common.h"

#define XBEE_SER Serial1

#define ULONG_MAX 4294967295L
#define NO_DISP 1000000

const int PIN_SW_CTRL = 2;   // Control key, lower left
const int PIN_SW_ALT = 3;   // Alt key (intermittent!)
const int PIN_SW_SHIFT = 4;   // Shift key, lower left
const int PIN_PWR = 11;
const int PIN_LED_BD = 13;

const int BUTTON_1L = 4;
const int BUTTON_1M = 7;
const int BUTTON_1R = 6;
const int BUTTON_2L = 1;
const int BUTTON_2M = 9;
const int BUTTON_2R = 3;
const int BUTTON_3L = 8;
const int BUTTON_3M = 2;
const int BUTTON_3R = 11;
const int BUTTON_4L = 5;
const int BUTTON_4M = 0;
const int BUTTON_4R = 10;

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
unsigned long msg6Time = 0UL;

// State bits;
boolean is2Upright = false;
boolean is2RunReady = false;
boolean is2Running = false;
boolean is2RouteInProgress = false;
boolean is6Upright = false;
boolean is6RunReady = false;
boolean is6Running = false;
boolean is6RouteInProgress = false;
boolean isFastMode = false;
boolean isJoySwap = false;

boolean isLightOn = false;

// Values to be displayed
boolean is2Connected = false; // for proper startup
boolean is6Connected = false; // for proper startup
float p2Heading = 0.0;
float p3Heading = 0.0;
float p2Fps = 0.0;
float p6Fps = 0.0;
int p2State = 0;
int p6State = 0;
int stateInt = 0;
float hcBatt = 0;
float p2Batt = 0;
float p6Batt = 0; 
int hcBattPct = 0;
int p2BattPct = 0;
int p6ABattPct = 0;
float v1 = 0.0;
float v2 = 0.0;
float p2SonarDistanceL = 0.0;
float p2SonarDistanceF = 0.0;
float p2SonarDistanceR = 0.0;
float p3SonarDistanceL = 0.0;
float p3SonarDistanceF = 0.0;
float p3SonarDistanceR = 0.0;
int tpMode = MODE_TP5;
int tpValSet = 99;
int tpBattVolt = 0;
int tpRouteStep = 0;
String p2Message = "";
String p3Message = "";

struct button {
  int pin;
  unsigned long releaseTime;
  boolean oldState;
  boolean isPressedTransition;
};
struct button buttons[] = {{34,0,false,false}, {35,0,false,false}, {36,0,false,false}, {37,0,false,false}, 
                           {38,0,false,false}, {39,0,false,false}, {40,0,false,false}, {41,0,false,false}, 
                           {42,0,false,false}, {43,0,false,false}, {44,0,false,false}, {45,0,false,false}};


/*******************************************************************************
 * setup()
 ******************************************************************************/
void setup() {
  XBEE_SER.begin(57600); // XBee
  Serial.begin(115200); // debug

  pinMode(PIN_SW_CTRL, INPUT_PULLUP);
  pinMode(PIN_SW_ALT, INPUT_PULLUP);
  pinMode(PIN_SW_SHIFT, INPUT_PULLUP);
  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_LED_BD, OUTPUT);
  digitalWrite(PIN_LED_BD, HIGH);
  digitalWrite(PIN_PWR, HIGH);
  for (int i = 0; i < 12; i++) pinMode(buttons[i].pin, INPUT_PULLUP);

  lcdInit();
}

/*******************************************************************************
 * check?Joystick()
 ******************************************************************************/
/*******************************************************************************
 *    loop() 
 ******************************************************************************/
void loop() {
  static unsigned int subCycle = 0;
  timeMilliseconds = millis();
  readXBee();
  buttonStates();
  if (timeMilliseconds > updateTrigger) { 
    checkButtons();
    updateTrigger = timeMilliseconds + 30;
    subCycle = ++subCycle % 3;
    if (subCycle == 0) { // Transmit to TwoPotatoe
        checkAJoystick();
        send2Potatoe();
    } else if (subCycle == 1) { // Transmit to SixPotoate
        checkBJoystick();
        send6Potatoe();
    } else { // Do misc tasks while 2&3Potatoe communicate
      lcdUpdate();
      checkConnected();
    }
  }
}



/*******************************************************************************
 * check?Joystick()
 ******************************************************************************/
void checkAJoystick() {
  const int JOY_ZERO = 15; // Range to treat as zero.

  // joystic A
  int a = analogRead(PIN_X_A);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  float jx = ((float) (a -512)) / 512.0; // scale to +- 1.0
  int b = analogRead(PIN_Y_A);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  float jy = ((float) (b -512)) / 512.0; // scale to +- 1.0
  if (!isFastMode) joyAY *= 0.4;
  if (!isJoySwap) {
    joyAX = jx;
    joyAY = jy;
  } else {
    joyBX = jx;
    joyBY = jy;
  }
  
}

void checkBJoystick() {
  const int JOY_ZERO = 15; // Range to treat as zero.
  
  // joystic B
  int a = 1024 - analogRead(PIN_X_B);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  float jx = ((float) (a -512)) / 512.0; // scale to +- 1.0
  int b = 1024 - analogRead(PIN_Y_B);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  float jy = ((float) (b -512)) / 512.0; // scale to +- 1.0
  if (!isJoySwap) {
    joyBX = jx;
    joyBY = jy;
  } else {
    joyAX = jx;
    joyAY = jy;
  }
}



/*******************************************************************************
 * checkButtons()
 ******************************************************************************/
void checkButtons() {
  int x = 0;
  boolean shift = (digitalRead(PIN_SW_SHIFT) == LOW) ? true : false;
  boolean ctrl = (digitalRead(PIN_SW_CTRL) == LOW) ? true : false;
  int alt = (digitalRead(PIN_SW_ALT) == LOW) ? true : false;

  if ((shift == false) && (ctrl == false)) {  // Shift & Control key not pressed
    if (hasPressed(BUTTON_1L)) queue2Msg(RCV_RUN, (is2RunReady) ? 0 : 1);
    if (hasPressed(BUTTON_1M)) queue2Msg(RCV_LIGHTS, 0);  // just toggle
    if (hasPressed(BUTTON_1R)) queue2Msg(RCV_LIFT, 0); // Toggle lift sensors
    if (hasPressed(BUTTON_2L)) queue2Msg(RCV_RT_START, 0);
    if (hasPressed(BUTTON_2M)) queue2Msg(RCV_RT_ENABLE, 0);
    if (hasPressed(BUTTON_3L)) queue6Msg(RCV_RUN, 0);  // Just toggle
    if (hasPressed(BUTTON_3M)) queue6Msg(RCV_GET_UP, 0);
    if (hasPressed(BUTTON_3R)) queue6Msg(RCV_LOG, 0);
    if (hasPressed(BUTTON_4L)) queue6Msg(RCV_RT_START, 0);
    if (hasPressed(BUTTON_4M)) queue6Msg(RCV_RT_ENABLE, 0);
  } else if ((shift == true) && (ctrl == false)) {  // Shift key pressed.
    if (hasPressed(BUTTON_1L)) queue2Msg(RCV_RT_SET, 1);
    if (hasPressed(BUTTON_1R)) isFastMode = ! isFastMode;
    if (hasPressed(BUTTON_2L)) queue2Msg(RCV_RT_SET, 0);
    if (hasPressed(BUTTON_2R)) isJoySwap = !isJoySwap;
    if (hasPressed(BUTTON_3L)) queue6Msg(RCV_RT_SET, 1);
    if (hasPressed(BUTTON_4L)) queue6Msg(RCV_RT_SET, 0);
    if (hasPressed(BUTTON_4R)) powerDown();
  } else if (ctrl == true) {
    if (hasPressed(BUTTON_1L)) queue2Msg(RCV_V1, 1);
    if (hasPressed(BUTTON_1M)) queue2Msg(RCV_V1, 0);
    if (hasPressed(BUTTON_1R)) queue2Msg(RCV_KILLTP, 0);
    if (hasPressed(BUTTON_2L)) queue2Msg(RCV_V2, 1);
    if (hasPressed(BUTTON_2M)) queue2Msg(RCV_V2, 0);
    if (hasPressed(BUTTON_2R)) queue2Msg(RCV_MOT_DISABLE, 0);
    if (hasPressed(BUTTON_3L)) queue6Msg(RCV_V1, 1);
    if (hasPressed(BUTTON_3M)) queue6Msg(RCV_V1, 0);
    if (hasPressed(BUTTON_4L)) queue6Msg(RCV_V2, 1);
    if (hasPressed(BUTTON_4M)) queue6Msg(RCV_V2, 0);
  } 
}


/*******************************************************************************
 * buttonStates()
 *      Polled frequently to check state of switches.
 ******************************************************************************/
void buttonStates() {
  int shift = digitalRead(PIN_SW_SHIFT);
  boolean state;

  for (int i = 0; i < 12; i++) {
    boolean isPressed = (digitalRead(buttons[i].pin) == LOW);
    if (isPressed) buttons[i].releaseTime = timeMilliseconds;
    state = ((timeMilliseconds - buttons[i].releaseTime) > 50) ? false : true;
    if (state && !buttons[i].oldState) buttons[i].isPressedTransition = true;
    buttons[i].oldState = state;
  }
}



/*******************************************************************************
 * powerDown()
 ******************************************************************************/
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



/*******************************************************************************
 * checkConnected()  Set the "isConnected" states'
 ******************************************************************************/
void checkConnected() {
  if (timeMilliseconds < 600) return;
  if ((msg2Time + 500) > timeMilliseconds) {
    is2Connected = true;
  } else {
    is2Connected = false;
  } if ((msg6Time + 500) > timeMilliseconds) {
    is6Connected = true;
  }
  else {
    is6Connected = false;
  }
}


/*******************************************************************************
 * interpret?State()  Set states from the "state" received from robot.
 ******************************************************************************/
void interpret2State(int state) {
  p2State = state;
  is2Running = (p2State & 1) > 0;
  is2RunReady = (p2State & 2) > 0;
  is2Upright = (p2State & 4) > 0;
  is2RouteInProgress = (p2State & 64) > 0;
}
void interpret6State(int state) {
  p6State = state;
  is6Running = (p6State & 1) > 0;
  is6RunReady = (p6State & 2) > 0;
  is6Upright = (p6State & 4) > 0;
  is6RouteInProgress = (p6State & 64) > 0;
}



/*******************************************************************************
 * hasPressed()  Return true if button has pressed transition.
 ******************************************************************************/
boolean hasPressed(int sw) {
  boolean ret = buttons[sw].isPressedTransition;
  if (ret) buttons[sw].isPressedTransition = false;
  return ret;
}

