#include "SensorManager.h"
#include <DHT.h>
#include "HX711.h"
#include <Arduino.h>
#include "Config.h"

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
HX711 scale;
long CALIBRATION_FACTOR = -4083333;

void initializeSensors()
{
  dht.begin();
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
}

SensorData readSensors()
{
  SensorData data;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.weight = -scale.get_units(5) * 400 * 100;
  if (data.weight < 0)
    data.weight = 0;
  data.limitSwitch = !digitalRead(LIMIT_SWITCH_PIN);
  data.filamentDiameter = 1.75;
  return data;
}