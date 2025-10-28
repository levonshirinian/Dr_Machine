#include "CommandHandler.h"
#include "HX711.h"
#include <ArduinoJson.h>

extern HX711 scale;
extern long CALIBRATION_FACTOR;

HardwareSerial& arduinoSerial = Serial1;
const int ARDUINO_RX_PIN = 16;
const int ARDUINO_TX_PIN = 17;

void initializeSerial() {
  Serial.begin(115200);     // USB Serial for debugging
  Serial1.begin(115200);    // Hardware Serial1 for communication with Arduino UNO
}

void handleIncomingCommands() {
  if (arduinoSerial.available() > 0 || Serial.available() > 0) {
    String jsonString;
    if (arduinoSerial.available()) {
      jsonString = arduinoSerial.readStringUntil('\n');
    } else {
      jsonString = Serial.readStringUntil('\n');
    }

    if (jsonString.startsWith("tare_scale")) {
      scale.tare();
      Serial.println("✅ Scale tared.");
    } else if (jsonString.startsWith("calibrate:")) {
      float knownWeight = jsonString.substring(jsonString.indexOf(':') + 1).toFloat();
      if (knownWeight > 0) {
        long raw = scale.read_average(20);
        if (raw != 0) {
          CALIBRATION_FACTOR = raw / knownWeight;
          scale.set_scale(CALIBRATION_FACTOR);
          Serial.print("✅ New factor: "); Serial.println(CALIBRATION_FACTOR);
        }
      }
    }
    // يمكنك إضافة أوامر إضافية هنا حسب الحاجة
  }
}