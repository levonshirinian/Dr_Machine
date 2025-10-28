#include "SessionManager.h"

void TestSession::addSample(float val, uint32_t ts) {
  samples[head] = {ts, val};
  head = (head + 1) % 10;
  if (count < 10) count++;
}

void TestSession::setEnvironment(float temp, float hum, float diameter) {
  temperature = temp;
  humidity = hum;
  thread_diameter = diameter;
}

void TestSession::setDateTime(String dt) {
  datetime = dt;
}

void TestSession::printSession() {
  Serial.println("📦 بيانات الجلسة:");
  Serial.print("🕒 التاريخ والوقت: "); Serial.println(datetime);
  Serial.print("🌡️ الحرارة: "); Serial.print(temperature, 1); Serial.println(" °C");
  Serial.print("💧 الرطوبة: "); Serial.print(humidity, 1); Serial.println(" %");
  Serial.print("🧵 قطر الخيط: "); Serial.print(humidity, 1); Serial.println(" mm");
  Serial.println("📈 القيم المقاسة:");
  for (int i = 0; i < count; i++) {
    int index = (head + i) % 10;
    Serial.print(" - زمن: ");
    Serial.print(samples[index].timestamp);
    Serial.print(" ms | القيمة: ");
    Serial.println(samples[index].value, 2);
  }
}

bool TestSession::shouldTakeSample() {
  unsigned long now = millis();
  if (now - lastSampleTime >= sampleInterval) {
    lastSampleTime = now;
    return true;
  }
  return false;
}

