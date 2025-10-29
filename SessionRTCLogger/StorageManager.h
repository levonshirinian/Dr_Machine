#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>

// Initializes the SD card with the given chip select pin
bool initializeSD(uint8_t csPin);

// Writes a single line of text to a file
bool writeToFile(const char* filename, const String& data);

// Logs a full session record in CSV format
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
);

// Reads the entire content of a file and returns it as a String
String readFile(const char* filename);

// Deletes a file from the SD card
bool deleteFile(const char* filename);

String listFiles();

#endif