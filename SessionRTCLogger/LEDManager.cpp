#include "LEDManager.h"
#include "BatteryManager.h"
#include "Config.h"
#include <Adafruit_NeoPixel.h>

// Create NeoPixel strip instance
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Blinking state variables
unsigned long previousBlinkMillis = 0;
const unsigned long blinkInterval = 500;
bool blinkState = false;

// Initializes the LED strip
bool initializeLEDs() {
  if (!strip.begin()) {
    return false;
  }
  strip.show();
  return true;
}

// Sets the LED to a specific RGB color
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

// Blinks the LED in orange
void blinkOrange() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    previousBlinkMillis = currentMillis;
    blinkState = !blinkState;
    if (blinkState) {
      setColor(255, 165, 0); // Orange
    } else {
      setColor(0, 0, 0);     // Off
    }
  }
}

// Updates LED color based on battery percentage
void updateLEDStatus() {
  float voltage = getBatteryVoltage(); // From BatteryManager
  int batteryPercent = getBatteryPercentage(); // From BatteryManager

  if (isBatteryCharging()) {
    blinkOrange();  // Orange blinking
  } else if (batteryPercent >= 95) {
    setColor(0, 255, 0); // Green
  } else if (batteryPercent <= 20) {
    setColor(255, 0, 0); // Red
  }
}

// Shows purple color for calibration mode
void showCalibrationMode() {
  setColor(0, 0, 128); // Dark Blue
}
