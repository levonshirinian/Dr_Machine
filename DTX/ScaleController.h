#ifndef SCALE_CONTROLLER_H
#define SCALE_CONTROLLER_H

#include <Arduino.h>
#include "HX711.h"

// External references to the HX711 instance and calibration factor
extern HX711 scale;
extern long CALIBRATION_FACTOR;

// Initializes the scale with default settings
void initializeScale();

// Tares the scale (resets the current weight to zero)
void tareScale();

// Calibrates the scale using a known weight
void calibrateScale(float knownWeight);

// Reads the current weight from the scale
float readWeight();

#endif