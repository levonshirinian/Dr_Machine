#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <Arduino.h>

struct Sample {
  uint32_t timestamp;
  float value;
};

class TestSession {
private:
  Sample samples[10];
  int head = 0;
  int count = 0;
  float temperature = 0.0;
  float humidity = 0.0;
  float thread_diameter = 0.0;
  String datetime;
  String material;

  unsigned long lastSampleTime = 0;
  const unsigned long sampleInterval = 1000;

public:
  void addSample(float val, uint32_t ts);
  void setEnvironment(String Material, float temp, float hum, float diameter);
  void setDateTime(String dt);
  void printSession();
  void saveSessionToStorage();
  bool shouldTakeSample(); 
};

#endif