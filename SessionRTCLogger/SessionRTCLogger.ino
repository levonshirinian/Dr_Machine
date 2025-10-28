#include "RTCManager.h"
#include "SessionManager.h"
#include "SensorManager.h"
#include "CommandHandler.h"
#include "DisplayManager.h"
#include "LEDManager.h"
#include "BatteryManager.h"

TestSession session;
bool isRecording = false;

void setup() {
  initializeSerial();
  initializeRTC();
  // initializeSensors();
  initializeDisplay();
  // initializeLEDs();
  // initializeBatteryMonitor(); // ← تهيئة مراقبة البطارية
  Serial.println("أرسل 'start' لبدء التسجيل أو 'end' لإنهائه");
}

void loop() {
  handleTouch();
  // handleIncomingCommands();
  // updateLEDs();

  // float voltage = getBatteryVoltage();
  // int percent = getBatteryPercentage();

  // Serial.print("Voltage: ");
  // Serial.print(voltage);
  // Serial.print(" V | Charge: ");
  // Serial.print(percent);
  // Serial.println(" %");

  // if (isRecording && session.shouldTakeSample()) {
  //   SensorData data = readSensors();
  //   session.addSample(data.weight, millis());
  //   session.setEnvironment(data.temperature, data.humidity, data.filamentDiameter);
  //   updateDisplay(data.temperature, data.humidity, data.weight, data.limitSwitch);
  // }
}