#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>

// Initializes the NeoPixel LED
bool initializeLEDs();

// Updates LED color based on battery percentage
void updateLEDStatus();

// Displays calibration mode color
void showCalibrationMode();

#endif