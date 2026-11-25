/*
 * ESP32 Dual Joystick BLE Gamepad Controller
 * ===========================================
 * Uses two analog joystick modules to create a Bluetooth gamepad
 * that connects to your laptop for playing games.
 *
 * Hardware:
 *   - ESP32 DevKit C V4
 *   - Joystick 1 (Left):  VRx->GPIO34, VRy->GPIO35, SW->GPIO32
 *   - Joystick 2 (Right): VRx->GPIO36, VRy->GPIO39, SW->GPIO33
 *
 * Library: ESP32-BLE-Gamepad by lemmingDev
 *   Install via Arduino Library Manager or:
 *   https://github.com/lemmingDev/ESP32-BLE-Gamepad
 *
 * For Wokwi simulation: BLE won't connect, but joystick values
 * are printed to Serial Monitor for verification.
 */

#include <BleGamepad.h>

// Pin Definitions
// Left Joystick (Joystick 1)
#define JOY1_X    34   // Analog - Left/Right
#define JOY1_Y    35   // Analog - Up/Down
#define JOY1_BTN  32   // Digital - L3 (Left Stick Click)

// Right Joystick (Joystick 2)
#define JOY2_X    36   // Analog - Left/Right
#define JOY2_Y    39   // Analog - Up/Down
#define JOY2_BTN  33   // Digital - R3 (Right Stick Click)

// Extra Buttons (optional, add physical buttons)
#define BTN_A     25   // Button A (South / Cross)
#define BTN_B     26   // Button B (East / Circle)
#define BTN_X     27   // Button X (West / Square)
#define BTN_Y     14   // Button Y (North / Triangle)
#define BTN_LB    12   // Left Bumper (L1)
#define BTN_RB    13   // Right Bumper (R1)

// Calibration
#define ADC_MIN       0
#define ADC_MAX       4095
#define ADC_CENTER    2048
#define DEADZONE      200

#define AXIS_MIN     -32767
#define AXIS_MAX      32767

// BLE Gamepad Instance
BleGamepad bleGamepad("ESP32 Dual Gamepad", "DIY Controller", 100);
BleGamepadConfiguration bleGamepadConfig;

// Variables
int joy1X, joy1Y, joy2X, joy2Y;
int mappedLX, mappedLY, mappedRX, mappedRY;
bool joy1Btn, joy2Btn;
bool btnA, btnB, btnX, btnY, btnLB, btnRB;

unsigned long lastPrint = 0;
const int PRINT_INTERVAL = 100;

int mapAxis(int adcValue) {
  if (abs(adcValue - ADC_CENTER) < DEADZONE) {
    return 0;
  }
  if (adcValue < ADC_CENTER) {
    return map(adcValue, ADC_MIN, ADC_CENTER - DEADZONE, AXIS_MIN, 0);
  } else {
    return map(adcValue, ADC_CENTER + DEADZONE, ADC_MAX, 0, AXIS_MAX);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Dual Joystick BLE Gamepad");
  Serial.println("Waiting for Bluetooth connection...");

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  pinMode(JOY1_BTN, INPUT_PULLUP);
  pinMode(JOY2_BTN, INPUT_PULLUP);
  pinMode(BTN_A,    INPUT_PULLUP);
  pinMode(BTN_B,    INPUT_PULLUP);
  pinMode(BTN_X,    INPUT_PULLUP);
  pinMode(BTN_Y,    INPUT_PULLUP);
  pinMode(BTN_LB,   INPUT_PULLUP);
  pinMode(BTN_RB,   INPUT_PULLUP);

  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepadConfig.setButtonCount(10);
  bleGamepadConfig.setHatSwitchCount(0);
  bleGamepadConfig.setWhichAxes(true, true, true, true, false, false, false, false);
  bleGamepadConfig.setAxesMin(AXIS_MIN);
  bleGamepadConfig.setAxesMax(AXIS_MAX);

  bleGamepad.begin(&bleGamepadConfig);

  Serial.println("BLE Gamepad started! Open Bluetooth settings on your laptop.");
  Serial.println("Look for 'ESP32 Dual Gamepad' and pair it.");
}

void loop() {
  joy1X = analogRead(JOY1_X);
  joy1Y = analogRead(JOY1_Y);
  joy2X = analogRead(JOY2_X);
  joy2Y = analogRead(JOY2_Y);

  mappedLX = mapAxis(joy1X);
  mappedLY = mapAxis(joy1Y);
  mappedRX = mapAxis(joy2X);
  mappedRY = mapAxis(joy2Y);

  joy1Btn = !digitalRead(JOY1_BTN);
  joy2Btn = !digitalRead(JOY2_BTN);
  btnA    = !digitalRead(BTN_A);
  btnB    = !digitalRead(BTN_B);
  btnX    = !digitalRead(BTN_X);
  btnY    = !digitalRead(BTN_Y);
  btnLB   = !digitalRead(BTN_LB);
  btnRB   = !digitalRead(BTN_RB);

  if (bleGamepad.isConnected()) {
    bleGamepad.setX(mappedLX);
    bleGamepad.setY(mappedLY);
    bleGamepad.setZ(mappedRX);
    bleGamepad.setRZ(mappedRY);

    if (btnA)    bleGamepad.press(BUTTON_1);  else bleGamepad.release(BUTTON_1);
    if (btnB)    bleGamepad.press(BUTTON_2);  else bleGamepad.release(BUTTON_2);
    if (btnX)    bleGamepad.press(BUTTON_3);  else bleGamepad.release(BUTTON_3);
    if (btnY)    bleGamepad.press(BUTTON_4);  else bleGamepad.release(BUTTON_4);
    if (btnLB)   bleGamepad.press(BUTTON_5);  else bleGamepad.release(BUTTON_5);
    if (btnRB)   bleGamepad.press(BUTTON_6);  else bleGamepad.release(BUTTON_6);
    if (joy1Btn) bleGamepad.press(BUTTON_7);  else bleGamepad.release(BUTTON_7);
    if (joy2Btn) bleGamepad.press(BUTTON_8);  else bleGamepad.release(BUTTON_8);

    bleGamepad.sendReport();
  }

  if (millis() - lastPrint >= PRINT_INTERVAL) {
    lastPrint = millis();
    Serial.print("L-Stick: X=");
    Serial.print(mappedLX);
    Serial.print(" Y=");
    Serial.print(mappedLY);
    Serial.print(" | R-Stick: X=");
    Serial.print(mappedRX);
    Serial.print(" Y=");
    Serial.print(mappedRY);
    Serial.print(" | BTNs: ");
    if (joy1Btn) Serial.print("L3 ");
    if (joy2Btn) Serial.print("R3 ");
    if (btnA)    Serial.print("A ");
    if (btnB)    Serial.print("B ");
    if (btnX)    Serial.print("X ");
    if (btnY)    Serial.print("Y ");
    if (btnLB)   Serial.print("LB ");
    if (btnRB)   Serial.print("RB ");
    Serial.print("| BLE: ");
    Serial.println(bleGamepad.isConnected() ? "CONNECTED" : "waiting...");
  }

  delay(10);
}
