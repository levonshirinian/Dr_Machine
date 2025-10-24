#include "RTCManager.h"
#include "SessionManager.h"
#include "SensorManager.h"
#include "CommandHandler.h"
#include "DisplayManager.h"

TestSession session;
bool isRecording = false;

void setup() {
  initializeSerial();
  // initializeRTC();
  // initializeSensors();
  initializeDisplay();
  // Serial.println("أرسل 'start' لبدء التسجيل أو 'end' لإنهائه");
}
int i = 0;
void loop() {
  handleTouch(); // ← لمس الشاشة
  // handleIncomingCommands(); // ← أوامر تسلسلية
  // if (isRecording && session.shouldTakeSample()) {
  //   SensorData data = readSensors();
  //   session.addSample(data.weight, millis());
  //   session.setEnvironment(data.temperature, data.humidity, data.filamentDiameter);
  //   updateDisplay(data.temperature, data.humidity, data.weight, data.limitSwitch);
  // }
}