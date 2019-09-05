/*****************************************************************************-
 *                        TPHandController
 *              Hand controller for TwoPotatoe and SixPotatoe
 *****************************************************************************/
#include "Ma_HC.h"

#define XBEE_SER Serial1

#define ULONG_MAX 4294967295L
#define NO_DISP 1000000

const int PIN_SW_CTRL = 3;   // Control key, lower left
const int PIN_SW_2_6 = 4;    // 2Potatoe & 6Potatoe modes
const int PIN_SW_SHIFT = 2;  // Shift key, lower left
const int PIN_PWR = 11;
const int PIN_LED_BD = 13;

const int BUTTON_1L_NUM = 4;
const int BUTTON_1M_NUM = 7;
const int BUTTON_1R_NUM = 6;
const int BUTTON_2L_NUM = 1;
const int BUTTON_2M_NUM = 9;
const int BUTTON_2R_NUM = 3;
const int BUTTON_3L_NUM = 8;
const int BUTTON_3M_NUM = 2;
const int BUTTON_3R_NUM = 11;
const int BUTTON_4L_NUM = 5;
const int BUTTON_4M_NUM = 0;
const int BUTTON_4R_NUM = 10;

const int PIN_X_A = A0;     // Gn Joystick A
const int PIN_Y_A = A1;     // Bu Joystick A
const int PIN_BATT = A2;  // Battery voltage
const int PIN_VKEY = A3;  // Vi Vkey switches
const int PIN_Y_B = A4;  // Joystick B
const int PIN_X_B = A5;  // Joystick B

boolean isP2Mode = true;  // true = TwoPotatoe, false = SixPotatoe mode
boolean isVMode = true;   // True if variables rather than steps are displayed.
unsigned long tStart = 0;

boolean lightState = false;
boolean routeState = false;
int key;
int oldKey = 99;
int oldShift = 99;
char message[100];

float joyX = 0.0;
float joyY = 0.0;

unsigned long timeMillis = 0L;
unsigned long updateTrigger = 0L;
unsigned long ledTrigger = 0L;
unsigned long timeLowBatt = ULONG_MAX;
unsigned long msg2Time = 0UL;
unsigned long msg6Time = 0UL;

boolean ctrlState = false;
boolean shiftState = false;

// State bits;
boolean is2Connected = false;
boolean is6Connected = false;
boolean is2Upright = false;
boolean is6Upright = false;
boolean is2RunReady = false;
boolean is6RunReady = false;
boolean is2Running = false;
boolean is6Running = false;
boolean is2RouteInProgress = false;
boolean is6RouteInProgress = false;

// Values to be displayed
int p2State = 0;
int p6State = 0;
float p2BattV = 0;
float p6BattV = 0;
float p2Fps = 0.0;
float p6Fps = 0.0;
int p2Tuning = 0;
int p6Tuning = 6;
String p2Message = "";
String p6Message = "";
int p2Step = 0;
int p6Step = 0;
float p2V1 = 0.0;
float p2V2 = 0.0;
float p6V1 = 0.0;
float p6V2 = 0.0;
float hcBattV = 0;

int hcBattPct = 0;
int p2BattPct = 0;
int p6BattPct = 0;

struct button {
  int pin;
  unsigned long releaseTime;
  boolean oldState;
  boolean isPressedTransition;
};
struct button buttons[] = {{34, 0, false, false}, {35, 0, false, false}, {36, 0, false, false}, {37, 0, false, false},
  {38, 0, false, false}, {39, 0, false, false}, {40, 0, false, false}, {41, 0, false, false},
  {42, 0, false, false}, {43, 0, false, false}, {44, 0, false, false}, {45, 0, false, false}
};


/*****************************************************************************-
 *   setup()
 ******************************************************************************/
void setup() {
  XBEE_SER.begin(57600); // XBee
  Serial.begin(115200); // debug

  pinMode(PIN_SW_CTRL, INPUT_PULLUP);
  pinMode(PIN_SW_2_6, INPUT_PULLUP);
  pinMode(PIN_SW_SHIFT, INPUT_PULLUP);
  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_LED_BD, OUTPUT);
  digitalWrite(PIN_LED_BD, HIGH);
  digitalWrite(PIN_PWR, HIGH);
  for (int i = 0; i < 12; i++) pinMode(buttons[i].pin, INPUT_PULLUP);

  lcdInit();
}



/*****************************************************************************-
 *    loop()
 ******************************************************************************/
void loop() {
  static unsigned int subCycle = 0;
  timeMillis = millis();
  readXBee();
  buttonStates();
  if (timeMillis > updateTrigger) {
    checkButtons();
    updateTrigger = timeMillis + 30;
    ++ subCycle;
    subCycle = subCycle % 3;
    if (subCycle == 0) { // Transmit to TwoPotatoe
      if (isP2Mode) checkAJoystick();
      else checkBJoystick();
      send2Potatoe();
    } else if (subCycle == 1) { // Transmit to SixPotoate
      if (isP2Mode) checkBJoystick();
      else checkAJoystick();
      send6Potatoe();
    } else { // Do misc tasks while 2&3Potatoe communicate
      lcdUpdate();
      checkConnected();
    }
  }
}



/*****************************************************************************-
 * check?Joystick()
 ******************************************************************************/
void checkAJoystick() {
  const int JOY_ZERO = 15; // Range to treat as zero.

  // joysticK A
  int a = analogRead(PIN_X_A);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  joyX = ((float) (a - 512)) / 512.0; // scale to +- 1.0
  int b = analogRead(PIN_Y_A);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
   joyY = ((float) (b - 512)) / 512.0; // scale to +- 1.0
}

void checkBJoystick() {
  const int JOY_ZERO = 15; // Range to treat as zero.

  // joysticK B
  int a = 1024 - analogRead(PIN_X_B);
  if ((a > (512 - JOY_ZERO)) && (a < (512 + JOY_ZERO))) a = 512;
  joyX = ((float) (a - 512)) / 512.0; // scale to +- 1.0
  int b = 1024 - analogRead(PIN_Y_B);
  if ((b > (512 - JOY_ZERO)) && (b < (512 + JOY_ZERO))) b = 512;
  joyY = ((float) (b - 512)) / 512.0; // scale to +- 1.0
}



/*****************************************************************************-
 * checkButtons()
 ******************************************************************************/
void checkButtons() {
  int bits = IS_PRESS_BIT;
  if (digitalRead(PIN_SW_SHIFT) == LOW) bits |= IS_SHIFT_BIT;
  if (digitalRead(PIN_SW_CTRL) == LOW) bits |= IS_CTRL_BIT;
  
  if (hasPressed(BUTTON_1L_NUM)) queueMsg(BUTTON_1L, bits);
  if (hasPressed(BUTTON_1M_NUM)) queueMsg(BUTTON_1M, bits);
  if (hasPressed(BUTTON_1R_NUM)) queueMsg(BUTTON_1R, bits);
  if (hasPressed(BUTTON_2L_NUM)) queueMsg(BUTTON_2L, bits);
  if (hasPressed(BUTTON_2M_NUM)) queueMsg(BUTTON_2M, bits);
  if (hasPressed(BUTTON_2R_NUM)) queueMsg(BUTTON_2R, bits);
  if (hasPressed(BUTTON_3L_NUM)) queueMsg(BUTTON_3L, bits);
  if (hasPressed(BUTTON_3M_NUM)) queueMsg(BUTTON_3M, bits);
  if (hasPressed(BUTTON_3R_NUM)) queueMsg(BUTTON_3R, bits);
  if (hasPressed(BUTTON_4L_NUM)) {
    queueMsg(BUTTON_4L, bits);
    if (bits == (IS_SHIFT_BIT | IS_PRESS_BIT)) powerDown();
  }
  if (hasPressed(BUTTON_4M_NUM)) queueMsg(BUTTON_4M, bits);
  if (hasPressed(BUTTON_4R_NUM)) queueMsg(BUTTON_4R, bits);
}
void queueMsg(unsigned int button, int bits) {
  int payload = (button * 256) + bits;
  if (isP2Mode) queue2Msg(RCV_BUTTON, payload);
  else queue6Msg(RCV_BUTTON, payload);
}

/*****************************************************************************-
 * buttonStates()
 *      Polled frequently to check state of switches.
 ******************************************************************************/
void buttonStates() {
  static unsigned long shiftTimer = 0;
  static unsigned long ctrlTimer = 0;
  static unsigned long m26Timer = 0;
  static boolean m26State = false;
  static boolean oldM26State = false;
  boolean state;

  for (int i = 0; i < 12; i++) {
    boolean isPressed = (digitalRead(buttons[i].pin) == LOW);
    if (isPressed) buttons[i].releaseTime = timeMillis;
    state = ((timeMillis - buttons[i].releaseTime) > 50) ? false : true;
    if (state && !buttons[i].oldState) buttons[i].isPressedTransition = true;
    buttons[i].oldState = state;
  }

  // Shift switch
  boolean isShiftPress = (digitalRead(PIN_SW_SHIFT) == LOW);
  if (isShiftPress) shiftTimer = timeMillis;
  shiftState = ((timeMillis - shiftTimer) > 50) ? false : true; 

  // Ctrl switch
  boolean isCtrlPressed = (digitalRead(PIN_SW_CTRL) == LOW);
  if (isCtrlPressed) ctrlTimer = timeMillis;
  ctrlState = ((timeMillis - ctrlTimer) > 50) ? false : true; 

  // Mode switch
  boolean ism26Pressed = (digitalRead(PIN_SW_2_6) == LOW);
  if (ism26Pressed) m26Timer = timeMillis;
  m26State = ((timeMillis - m26Timer) > 50) ? false : true; 
  if (m26State && (!oldM26State)) isP2Mode = !isP2Mode;
  oldM26State = m26State;
}



/*****************************************************************************-
 * powerDown()
 ******************************************************************************/
void powerDown() {
  digitalWrite(PIN_PWR, LOW);
  Serial.println("PowerDown");
}



void ledBlink() {
  if (ledTrigger < timeMillis) { // once/second
    ledTrigger = timeMillis + 500;
    digitalWrite(PIN_LED_BD, !digitalRead(PIN_LED_BD));
  }
} // end ledBlink()



/*****************************************************************************-
 * checkConnected()  Set the "isConnected" states'
 ******************************************************************************/
void checkConnected() {
  if (timeMillis < 600) return;
  
  if ((msg2Time + 500) > timeMillis) {
    is2Connected = true;
  } else {
    is2Connected = false;
  } 
  
  if ((msg6Time + 500) > timeMillis) {
    is6Connected = true;
  } else {
    is6Connected = false;
  }
}


/*****************************************************************************-
 *  interpret?State()  Set states from the "state" received from robot.
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



/*****************************************************************************-
 *  hasPressed()  Return true if button has pressed transition.
 ******************************************************************************/
boolean hasPressed(int sw) {
  boolean ret = buttons[sw].isPressedTransition;
  if (ret) buttons[sw].isPressedTransition = false;
  return ret;
}
