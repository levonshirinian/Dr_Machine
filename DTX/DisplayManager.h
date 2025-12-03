#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

void initializeDisplay();
void updateDisplay(float temp, float hum, float weight, bool limitState);
String getCurrentMaterial();
char handleTouch();

#endif