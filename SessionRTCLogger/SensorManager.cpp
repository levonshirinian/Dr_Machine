#include "SensorManager.h"
#include <DHT.h>
#include "HX711.h"
#include <Arduino.h>

#define DHTPIN 32
#define DHTTYPE DHT11
const int LOADCELL_DOUT_PIN = 27;
const int LOADCELL_SCK_PIN = 26;
const int LIMIT_SWITCH_PIN = 25;

DHT dht(DHTPIN, DHTTYPE);
HX711 scale;
long CALIBRATION_FACTOR = -4083333;

void initializeSensors() {
  dht.begin();
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
}

SensorData readSensors() {
  SensorData data;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.weight = -scale.get_units(5) * 400 * 100;
  data.limitSwitch = !digitalRead(LIMIT_SWITCH_PIN);
  data.filamentDiameter = 1.75; // ثابت أو يمكن قراءته لاحقًا من حساس
  return data;
}