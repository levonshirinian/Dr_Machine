#include "Config.h"
#include "RTCManager.h"
#include "SessionManager.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "LEDManager.h"
#include "BatteryManager.h"
#include "StorageManager.h"
#include "ScaleController.h"


TestSession session;
bool isRecording = false;

void setup() {
  initializeRTC();
  if (initializeSD(31)) {
    Serial.println("SD جاهز");
  } else {
    Serial.println("فشل في تهيئة SD");
  }

  initializeSensors();
  initializeDisplay();
  initializeLEDs();
  initializeBatteryMonitor();

  initializeSD(10);  // تهيئة البطاقة

  float samples[10] = { 12.5, 13.0, 12.8, 13.1, 12.9, 13.2, 13.0, 12.7, 13.3, 13.0 };
  appendSession(
    1,
    "cotton",
    1.75,
    55.2,
    28.5,
    "2025/10/30 14:42",
    12.95,
    12.5,
    13.3,
    10,
    1,
    samples,
    10);

  Serial.println("أرسل 'start' لبدء التسجيل أو 'end' لإنهائه");
}

void loop() {
  handleTouch();
  updateLEDs();

  float voltage = getBatteryVoltage();
  int percent = getBatteryPercentage();

  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V | Charge: ");
  Serial.print(percent);
  Serial.println(" %");

  if (isRecording && session.shouldTakeSample()) {
    SensorData data = readSensors();
    session.addSample(data.weight, millis());
    session.setEnvironment(data.temperature, data.humidity, data.filamentDiameter);
    updateDisplay(data.temperature, data.humidity, data.weight, data.limitSwitch);
  }
}