#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>

void initializeBatteryMonitor();
float getBatteryVoltage();
int getBatteryPercentage();
bool isBatteryCharging();
#endif