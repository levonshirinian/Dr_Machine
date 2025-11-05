#include "BatteryManager.h"
#include "Config.h"

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

float previousVoltage = 0.0;

bool isBatteryCharging() {
  float currentVoltage = getBatteryVoltage();
  bool charging = currentVoltage > previousVoltage + 0.02; // فرق بسيط يدل على الشحن
  previousVoltage = currentVoltage;
  return charging;
}
