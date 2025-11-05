#ifndef CONFIG_H
#define CONFIG_H

// -----------------------------
// Pin Definitions
// -----------------------------
#define LOADCELL_DOUT_PIN      5   // HX711 data pin
#define LOADCELL_SCK_PIN       6   // HX711 clock pin

#define TARE_BUTTON_PIN        2   // Button to tare the scale
#define CALIBRATE_BUTTON_PIN   3   // Button to calibrate the scale
#define READ_BUTTON_PIN        4   // Button to read weight

#define LED_PIN         22         // NeoPixel LED pin
#define NUM_LEDS        1          // Number of NeoPixel LEDs
#define TEST_BUTTON_PIN 2

#define batteryPin A8              // Battery voltage ADC pin

// -----------------------------
// Calibration Settings
// -----------------------------
#define DEFAULT_CALIBRATION_FACTOR 420.0  // Initial calibration factor

// -----------------------------
// Display Settings (example pins)
// -----------------------------
#define TFT_CS   10   // Chip select
#define TFT_RST  9    // Reset
#define TFT_DC   8    // Data/command

// -----------------------------
// Session Settings
// -----------------------------
#define MAX_SESSION_SAMPLES 40   // Maximum number of samples per session

#endif