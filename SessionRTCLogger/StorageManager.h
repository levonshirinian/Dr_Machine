#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>

// Initializes SD card and ensures the session file exists with header and test data
bool initializeSD(uint8_t csPin);

// Appends a session record to the file
bool appendSession(
  int id,
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

// Reads a session line by ID
String getSessionById(int targetId);

// Deletes a session by its ID
bool deleteSessionById(int targetId);

// Returns the number of stored sessions (excluding header)
int getSessionCount();

// Reads the full content of the session file
String readAllSessions();

// Deletes all session data (clears the file and keeps header)
bool clearSessionFile();

// Returns a list of all session IDs found in the file
void getAllSessionIds(int* idList, int& count);

#endif