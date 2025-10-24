#include "RTCManager.h"
#include <Wire.h>

RTC_DS3231 rtc;

void initializeRTC() {
#if defined(ESP32)
  Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22
#else
  Wire.begin();       // SDA = A4, SCL = A5
#endif

  if (!rtc.begin()) {
    Serial.println("❌ لم يتم العثور على وحدة RTC");
    // while (1);
  }

  // استخدم هذا السطر مرة واحدة فقط لضبط الوقت حسب وقت الكومبايل
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

String getFormattedDateTime() {
  DateTime now = rtc.now();
  char buffer[30];
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
          now.day(), now.month(), now.year(),
          now.hour(), now.minute(), now.second());
  return String(buffer);
}