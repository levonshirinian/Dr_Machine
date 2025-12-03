#include "SessionManager.h"
#include "StorageManager.h"

void TestSession::addSample(float val, uint32_t ts) {
  samples[head] = {ts, val};
  head = (head + 1) % 10;
  if (count < 10) count++;
}

void TestSession::setEnvironment(String Material, float temp, float hum, float diameter) {
  material = Material;
  temperature = temp;
  humidity = hum;
  thread_diameter = diameter;
}

void TestSession::setDateTime(String dt) {
  datetime = dt;
}

void TestSession::printSession() {
  Serial.println("📦 Session Data:");
  Serial.print("🕒 Date & Time: "); Serial.println(datetime);
  Serial.print("🌡️ Temperature: "); Serial.print(temperature, 1); Serial.println(" °C");
  Serial.print("💧 Humidity: "); Serial.print(humidity, 1); Serial.println(" %");
  Serial.print("🧵 Thread Diameter: "); Serial.print(thread_diameter, 1); Serial.println(" mm");
  Serial.println("📈 Measured Values:");
  for (int i = 0; i < count; i++) {
    int index = (head + i) % 10;
    Serial.print("Index: ");
    Serial.print(index);
    Serial.print(" - Time: ");
    Serial.print(samples[index].timestamp);
    Serial.print(" ms | Value: ");
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

void TestSession::saveSessionToStorage() {
  float samplesArray[10];
  for (int i = 0; i < count; i++) {
    int index = (head + i) % 10;
    samplesArray[i] = samples[index].value;
  }

  float averageValue = 0.0;
  float minValue = 0.0;
  float maxValue = 0.0;

  if (count > 0) {
    minValue = samplesArray[0];
    maxValue = samplesArray[0];
    for (int i = 0; i < count; i++) {
      averageValue += samplesArray[i];
      if (samplesArray[i] < minValue) minValue = samplesArray[i];
      if (samplesArray[i] > maxValue) maxValue = samplesArray[i];
    }
    averageValue /= count;
  }

  appendSession(
    material,
    thread_diameter,
    humidity,
    temperature,
    datetime,
    averageValue, // average placeholder
    minValue,     // min placeholder
    maxValue,     // max placeholder
    count,
    head,
    samplesArray,
    count
  );
}
