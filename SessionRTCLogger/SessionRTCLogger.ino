#include "Config.h"
#include "RTCManager.h"
#include "SessionManager.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "LEDManager.h"
#include "BatteryManager.h"
#include "StorageManager.h"
#include "ScaleController.h"

TestSession session;
bool isRecording = false;
bool isShouldSaved = false;

void setup()
{
  Serial.begin(115200);
  if (initializeRTC())
  {
    Serial.println("RTC initialized successfully");
  }
  else
  {
    Serial.println("Failed to initialize RTC");
  }
  if (initializeSD(53))
  {
    Serial.println("SD initialized successfully");
  }
  else
  {
    Serial.println("Failed to initialize SD");
  }
  if (initializeLEDs())
  {
    Serial.println("LED initialized successfully");
  }
  else
  {
    Serial.println("Failed to initialize LED");
  }
  initializeDisplay();
  initializeBatteryMonitor();
  initializeSensors();
  Serial.println("Welcome DTX");
}

void loop()
{
  handleTouch();
  float voltage = getBatteryVoltage();
  int percent = getBatteryPercentage();
  SensorData data = readSensors();
  String material = getCurrentMaterial();

  updateLEDStatus();
  session.setDateTime(getFormattedDateTime());
  updateDisplay(data.temperature, data.humidity, data.weight, data.limitSwitch);
  isRecording = data.limitSwitch;
  if (isRecording)
  {
    showCalibrationMode();
  }
  else if (isShouldSaved)
  {
    session.printSession();
    session.saveSessionToStorage();
    isShouldSaved = false;
  }
  if (isRecording && session.shouldTakeSample())
  {
    // Serial.println(getFormattedDateTime());
    // Serial.print("Voltage: ");
    // Serial.print(voltage);
    // Serial.print(" V | Charge: ");
    // Serial.print(percent);
    // Serial.println(" %");
    // Serial.print("temperature: ");
    // Serial.print(data.temperature);
    // Serial.print(" humidity: ");
    // Serial.print(data.humidity);
    // Serial.print(" filamentDiameter: ");
    // Serial.print(data.filamentDiameter);
    // Serial.print(" weight: ");
    // Serial.println(data.weight);
    session.addSample(data.weight, millis());
    session.setEnvironment(material, data.temperature, data.humidity, data.filamentDiameter);
    isShouldSaved = true;
  }
}