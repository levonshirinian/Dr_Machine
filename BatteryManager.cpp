#include "BatteryManager.h"

const int batteryPin = A0;  // مدخل ADC
float voltage = 0.0;
int batteryPercent = 0;

void initializeBatteryMonitor() {
  pinMode(batteryPin, INPUT);
}

float getBatteryVoltage() {
  int raw = analogRead(batteryPin);
  voltage = (raw / 1023.0) * 5.0 * 2;  // ضرب بـ2 إذا كنت تستخدم مقسم جهد
  return voltage;
}

int getBatteryPercentage() {
  float v = getBatteryVoltage();
  batteryPercent = map(v * 100, 300, 420, 0, 100);
  batteryPercent = constrain(batteryPercent, 0, 100);
  return batteryPercent;
}