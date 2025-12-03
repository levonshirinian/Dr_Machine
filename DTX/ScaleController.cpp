#include "Config.h"
#include "ScaleController.h"

// Initializes the HX711 scale with the defined pins and calibration factor
void initializeScale() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();
}

// Resets the scale to zero
void tareScale() {
  scale.tare();
}

// Calculates and sets a new calibration factor based on a known weight
void calibrateScale(float knownWeight) {
  long raw = scale.read_average(20);
  if (raw != 0 && knownWeight > 0) {
    CALIBRATION_FACTOR = raw / knownWeight;
    scale.set_scale(CALIBRATION_FACTOR);
  }
}

// Returns the current weight reading from the scale
float readWeight() {
  return scale.get_units();
}