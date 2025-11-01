#include "StorageManager.h"
#include <SD.h>

const char *sessionFile = "sessions.csv";

bool initializeSD(uint8_t csPin)
{
  if (!SD.begin(csPin))
    return false;

  if (!SD.exists(sessionFile))
  {
    File file = SD.open(sessionFile, FILE_WRITE);
    if (!file)
      return false;

    file.println("id,material,diameter,humidity,temperature,dateTime,average,min,max,count,head,sample0,sample1,sample2,sample3,sample4,sample5,sample6,sample7,sample8,sample9");
    file.println("0,test,1.75,50.0,25.0,2025/10/30 14:00,12.5,10.0,15.0,10,1,12.5,13.0,12.8,13.1,12.9,13.2,13.0,12.7,13.3,13.0");
    file.close();
  }

  return true;
}

bool appendSession(
    int id,
    const String &material,
    float diameter,
    float humidity,
    float temperature,
    const String &dateTime,
    float average,
    float minValue,
    float maxValue,
    int count,
    int head,
    const float samples[],
    int sampleCount)
{
  File file = SD.open(sessionFile, FILE_WRITE);
  if (!file)
    return false;

  String line = "";
  line += String(id) + ",";
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

  for (int i = 0; i < 10; i++)
  {
    if (i < sampleCount)
    {
      line += "," + String(samples[i], 2);
    }
    else
    {
      line += ",";
    }
  }

  file.println(line);
  file.close();
  return true;
}

String getSessionById(int targetId)
{
  File file = SD.open(sessionFile);
  if (!file)
    return "ERROR: File not found";

  bool skipHeader = true;
  while (file.available())
  {
    String line = file.readStringUntil('\n');
    if (skipHeader)
    {
      skipHeader = false;
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1)
      continue;

    int id = line.substring(0, commaIndex).toInt();
    if (id == targetId)
    {
      file.close();
      return line;
    }
  }

  file.close();
  return "NOT FOUND";
}

bool deleteSessionById(int targetId)
{
  if (!SD.exists(sessionFile))
    return false;

  File original = SD.open(sessionFile);
  if (!original)
    return false;

  String lines[128];
  int count = 0;

  while (original.available())
  {
    String line = original.readStringUntil('\n');
    if (count == 0)
    {
      lines[count++] = line; // keep header
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1)
      continue;

    int id = line.substring(0, commaIndex).toInt();
    if (id != targetId)
    {
      lines[count++] = line;
    }
  }

  original.close();
  SD.remove(sessionFile);

  File updated = SD.open(sessionFile, FILE_WRITE);
  if (!updated)
    return false;

  for (int i = 0; i < count; i++)
  {
    updated.println(lines[i]);
  }

  updated.close();
  return true;
}

int getSessionCount()
{
  File file = SD.open(sessionFile);
  if (!file)
    return 0;

  int count = 0;
  bool skipHeader = true;

  while (file.available())
  {
    String line = file.readStringUntil('\n');
    if (skipHeader)
    {
      skipHeader = false;
      continue;
    }
    if (line.length() > 0)
      count++;
  }

  file.close();
  return count;
}

String readAllSessions()
{
  File file = SD.open(sessionFile);
  if (!file)
    return "ERROR: File not found";

  String content = "";
  while (file.available())
  {
    content += file.readStringUntil('\n') + "\n";
  }

  file.close();
  return content;
}

void getAllSessionIds(int *idList, int &count)
{
  count = 0;
  File file = SD.open(sessionFile);
  if (!file)
    return;

  bool skipHeader = true;
  while (file.available())
  {
    String line = file.readStringUntil('\n');
    if (skipHeader)
    {
      skipHeader = false;
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1)
      continue;

    String idStr = line.substring(0, commaIndex);
    idStr.trim(); // remove any spaces
    int id = idStr.toInt();

    if (id >= 0)
    {
      idList[count++] = id;
      if (count >= 128)
        break; // prevent overflow
    }
  }

  file.close();
}

bool clearSessionFile()
{
  if (SD.exists(sessionFile))
  {
    SD.remove(sessionFile);
    File file = SD.open(sessionFile, FILE_WRITE);
    if (!file)
      return false;
    file.println("id,material,diameter,humidity,temperature,dateTime,average,min,max,count,head,sample0,sample1,sample2,sample3,sample4,sample5,sample6,sample7,sample8,sample9");
    file.close();
    return true;
  }
  return false;
}