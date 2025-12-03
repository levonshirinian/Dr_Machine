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
char current_screen = 'T';
bool isRecording = false;
bool isShouldSaved = false;

void setup()
{
  Serial.begin(115200);
  Serial.println(initializeRTC() ? "RTC initialized successfully" : "Failed to initialize RTC");
  Serial.println(initializeSD() ? "SD initialized successfully" : "Failed to initialize SD");
  Serial.println(initializeLEDs() ? "LED initialized successfully" : "Failed to initialize LED");
  initializeDisplay();
  initializeBatteryMonitor();
  initializeSensors();
  Serial.println("Welcome DTX");
}

void loop()
{
  float voltage = getBatteryVoltage();
  int percent = getBatteryPercentage();
  SensorData data = readSensors();
  current_screen = handleTouch();
  String material = getCurrentMaterial();

  updateLEDStatus();
  session.setDateTime(getFormattedDateTime());
  updateDisplay(data.temperature, data.humidity, data.weight, data.limitSwitch);
  isRecording = data.limitSwitch;

  Serial.println(getFormattedDateTime());
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V | Charge: ");
  Serial.print(percent);
  Serial.println(" %");

  Serial.print("temperature: ");
  Serial.print(data.temperature);
  Serial.print(" humidity: ");
  Serial.print(data.humidity);
  Serial.print(" filamentDiameter: ");
  Serial.print(data.filamentDiameter);
  Serial.print(" weight: ");
  Serial.println(data.weight);

  if (isRecording && current_screen != 'M')
  {
    showCalibrationMode();
  }
  else if (isShouldSaved)
  {
    session.printSession();
    session.saveSessionToStorage();
    isShouldSaved = false;
  }
  if (isRecording && session.shouldTakeSample() && current_screen == 'T')
  {
    session.addSample(data.weight, millis());
    session.setEnvironment(material, data.temperature, data.humidity, data.filamentDiameter);
    isShouldSaved = true;
  }
}