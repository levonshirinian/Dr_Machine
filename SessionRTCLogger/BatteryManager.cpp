#include "BatteryManager.h"

const int batteryPin = A8;  // مدخل ADC
float voltage = 0.0;
int batteryPercent = 0;

void initializeBatteryMonitor() {
  pinMode(batteryPin, INPUT);
}

float getBatteryVoltage() {
  int raw = analogRead(batteryPin);
  voltage = raw;
  voltage = (map(raw, 0, 1023, 0, 5000)) / 1000.0;
  return voltage;
}

int getBatteryPercentage() {
  float v = getBatteryVoltage();
  batteryPercent = map(v * 100, 300, 420, 0, 100);
  batteryPercent = constrain(batteryPercent, 0, 100);
  return batteryPercent;
}