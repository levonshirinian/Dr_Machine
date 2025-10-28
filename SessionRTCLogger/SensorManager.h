#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

struct SensorData {
  float temperature;
  float humidity;
  float weight;
  float filamentDiameter;
  bool limitSwitch;
};

void initializeSensors();
SensorData readSensors();

#endif