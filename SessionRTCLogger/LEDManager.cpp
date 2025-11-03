#include "LEDManager.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 22
#define NUM_LEDS 1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0;
const long interval = 150;  // مدة التغيير بالمللي ثانية

bool initializeLEDs() {
  if (!strip.begin()) {
    return false;
  }
  strip.show();
  randomSeed(analogRead(0));
  return true;
}

void updateLEDs() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 0; i < NUM_LEDS; i++) {
      uint8_t r = random(0, 256);
      uint8_t g = random(0, 256);
      uint8_t b = random(0, 256);
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
  }
}