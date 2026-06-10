/*
 *  G25 Pedals + Handbrake — USB HID Joystick
 *  Based on DMadison's Sim Racing Library examples:
 *    PedalsJoystick.ino / HandbrakeJoystick.ino
 *
 *  Pins:
 *    A0 → Throttle   (Rx axis)
 *    A1 → Brake      (Ry axis)
 *    A2 → Clutch     (Rz axis)
 *    A3 → Handbrake  (Throttle slider)
 */

#include <SimRacing.h>
#include <Joystick.h>

// ============================================================
//  DEBUG MODE
//  true  = serial output only, joystick axes are NOT sent
//  false = normal USB HID operation, no serial output
// ============================================================
#define DEBUG_MODE false

// --- Pins (matching your original sketch) ---
const int PIN_THROTTLE  = A0;
const int PIN_BRAKE     = A1;
const int PIN_CLUTCH    = A2;
const int PIN_HANDBRAKE = A3;

// --- SimRacing objects ---
SimRacing::LogitechPedals pedals(PIN_THROTTLE, PIN_BRAKE, PIN_CLUTCH);
SimRacing::Handbrake       handbrake(PIN_HANDBRAKE);

// --- Joystick (same axis layout as your original sketch) ---
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  0, 0,           // no buttons, no hat switches
  false, false,   // no X / Y
  false,          // no Z
  true,           // Rx → Throttle
  true,           // Ry → Brake
  true,           // Rz → Clutch
  false,          // no rudder
  true,           // Throttle slider → Handbrake
  false, false, false
);

const int ADC_Max     = 1023;
const bool AlwaysSend = false;  // set true to send every loop regardless of changes

// ============================================================

void setup() {
#if DEBUG_MODE
  Serial.begin(115200);
  while (!Serial);
  Serial.println("=== DEBUG MODE — joystick output disabled ===");
  Serial.println("        --- RAW ---              --- MAPPED (0-1023) ---");
  Serial.println("THR_R   BRK_R   CLT_R   HB_R  | THR_M   BRK_M   CLT_M   HB_M");
  Serial.println("-------------------------------------------------------------");
#else
  Joystick.begin(false);  // disable auto-send; we call sendState() manually
  Joystick.setRxAxisRange(0, ADC_Max);
  Joystick.setRyAxisRange(0, ADC_Max);
  Joystick.setRzAxisRange(0, ADC_Max);
  Joystick.setThrottleRange(0, ADC_Max);
#endif

  pedals.begin();
  handbrake.begin();

  // ---- Calibration (optional) ----
  // Uncomment and adjust values after running DEBUG_MODE to find your min/max.
  //
  // pedals.setCalibration(SimRacing::Gas,    { 50, 950 });
  // pedals.setCalibration(SimRacing::Brake,  { 50, 950 });
  // pedals.setCalibration(SimRacing::Clutch, { 50, 950 });
  // handbrake.setCalibration({ 50, 950 });
pedals.setCalibration(
  { 980, 125 },  // Gas
  { 955,   5 },  // Brake
  { 982,  70 }   // Clutch
);
handbrake.setCalibration({ 1023, 0 });

#if !DEBUG_MODE
  updateJoystick();  // send initial state on connect
#endif
}

void loop() {
  pedals.update();
  handbrake.update();

#if DEBUG_MODE
  static unsigned long lastDebugTime = 0;
  const  unsigned long DEBUG_INTERVAL = 100;

  unsigned long now = millis();
  if (now - lastDebugTime >= DEBUG_INTERVAL) {
    lastDebugTime = now;

    int rawThrottle  = pedals.getPositionRaw(SimRacing::Gas);
    int rawBrake     = pedals.getPositionRaw(SimRacing::Brake);
    int rawClutch    = pedals.getPositionRaw(SimRacing::Clutch);
    int rawHandbrake = handbrake.getPositionRaw();

    int mThrottle  = pedals.getPosition(SimRacing::Gas,    0, ADC_Max);
    int mBrake     = pedals.getPosition(SimRacing::Brake,  0, ADC_Max);
    int mClutch    = pedals.getPosition(SimRacing::Clutch, 0, ADC_Max);
    int mHandbrake = handbrake.getPosition(0, ADC_Max);

    Serial.print(rawThrottle);  Serial.print("\t");
    Serial.print(rawBrake);     Serial.print("\t");
    Serial.print(rawClutch);    Serial.print("\t");
    Serial.print(rawHandbrake); Serial.print("\t| ");
    Serial.print(mThrottle);    Serial.print("\t");
    Serial.print(mBrake);       Serial.print("\t");
    Serial.print(mClutch);      Serial.print("\t");
    Serial.println(mHandbrake);
  }
#else
  if (pedals.positionChanged() || handbrake.positionChanged() || AlwaysSend) {
    updateJoystick();
  }
#endif
}

void updateJoystick() {
  Joystick.setRxAxis(pedals.getPosition(SimRacing::Gas,    0, ADC_Max));
  Joystick.setRyAxis(pedals.getPosition(SimRacing::Brake,  0, ADC_Max));
  Joystick.setRzAxis(pedals.getPosition(SimRacing::Clutch, 0, ADC_Max));
  Joystick.setThrottle(handbrake.getPosition(0, ADC_Max));
  Joystick.sendState();
}
