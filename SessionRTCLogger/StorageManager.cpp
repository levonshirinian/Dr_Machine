#include "StorageManager.h"
#include <SD.h>

// Initializes the SD card
bool initializeSD(uint8_t csPin) {
  return SD.begin(csPin);
}

// Writes a line of text to the specified file
bool writeToFile(const char* filename, const String& data) {
  File file = SD.open(filename, FILE_WRITE);
  if (!file) return false;
  file.println(data);
  file.close();
  return true;
}

// Logs session data in CSV format
bool logSessionData(
  const char* filename,
  const String& material,
  float diameter,
  float humidity,
  float temperature,
  const String& dateTime,
  float average,
  float minValue,
  float maxValue,
  int count,
  int head,
  const float samples[],
  int sampleCount
) {
  String line = "";
  line += material + ",";
  line += String(diameter, 2) + ",";
  line += String(humidity, 2) + ",";
  line += String(temperature, 2) + ",";
  line += dateTime + ",";
  line += String(average, 2) + ",";
  line += String(minValue, 2) + ",";
  line += String(maxValue, 2) + ",";
  line += String(count) + ",";
  line += String(head);

  for (int i = 0; i < sampleCount; i++) {
    line += "," + String(samples[i], 2);
  }

  return writeToFile(filename, line);
}

// Reads the entire content of a file
String readFile(const char* filename) {
  File file = SD.open(filename);
  if (!file) return "ERROR: File not found";

  String content = "";
  while (file.available()) {
    content += file.readStringUntil('\n') + "\n";
  }

  file.close();
  return content;
}

// Deletes a file from the SD card
bool deleteFile(const char* filename) {
  if (SD.exists(filename)) {
    return SD.remove(filename);
  }
  return false;
}

String listFiles() {
  String result = "";
  File root = SD.open("/");

  if (!root) return "SD not accessible.";
  if (!root.isDirectory()) return "SD root is not a directory.";

  File entry = root.openNextFile();
  while (entry) {
    result += String(entry.name()) + "\n";
    entry.close();
    entry = root.openNextFile();
  }

  return result;
}
