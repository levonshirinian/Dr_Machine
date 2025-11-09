#include "StorageManager.h"
#include <SdFat.h>
#include "config.h"

const char *sessionFile = "sessions.csv";

SdFat sd;
FsVolume volume;

bool initializeSD() {
  if (!sd.begin(SD_CS_PIN)) return false;

  if (!sd.exists(sessionFile)) {
    SdFile file;
    if (!file.open(sessionFile, O_WRITE | O_CREAT | O_APPEND)) return false;

    file.println("id,material,diameter,humidity,temperature,dateTime,average,min,max,count,head,sample0,sample1,sample2,sample3,sample4,sample5,sample6,sample7,sample8,sample9");
    file.println("0,test,1.75,50.0,25.0,2001/9/11 14:00,12.5,10.0,15.0,10,1,12.5,13.0,12.8,13.1,12.9,13.2,13.0,12.7,13.3,13.0");
    file.flush();
    file.close();
  }

  return true;
}

bool appendSession(
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
    int sampleCount) {

  SdFile file;
  if (!file.open(sessionFile, O_WRITE | O_CREAT | O_APPEND)) return false;

  int lastId = getLastSessionId();
  if (lastId == -1) lastId = 0;

  String line = "";
  line += String(lastId + 1) + ",";
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

  for (int i = 0; i < 10; i++) {
    if (i < sampleCount) {
      line += "," + String(samples[i], 2);
    } else {
      line += ",";
    }
  }

  file.println(line);
  file.flush();
  file.close();
  return true;
}

String getSessionById(int targetId) {
  SdFile file;
  if (!file.open(sessionFile, O_READ)) return "ERROR: File not found";

  bool skipHeader = true;
  char lineBuffer[256];

  while (file.available()) {
    file.fgets(lineBuffer, sizeof(lineBuffer));
    String line = String(lineBuffer);
    line.trim();

    if (skipHeader) {
      skipHeader = false;
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1) continue;

    int id = line.substring(0, commaIndex).toInt();
    if (id == targetId) {
      file.close();
      return line;
    }
  }

  file.close();
  return "NOT FOUND";
}

bool deleteSessionById(int targetId) {
  if (!sd.exists(sessionFile)) return false;

  SdFile original;
  if (!original.open(sessionFile, O_READ)) return false;

  String lines[128];
  int count = 0;
  char lineBuffer[256];

  while (original.available()) {
    original.fgets(lineBuffer, sizeof(lineBuffer));
    String line = String(lineBuffer);
    line.trim();

    if (count == 0) {
      lines[count++] = line;
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1) continue;

    int id = line.substring(0, commaIndex).toInt();
    if (id != targetId) {
      lines[count++] = line;
    }
  }

  original.close();
  sd.remove(sessionFile);

  SdFile updated;
  if (!updated.open(sessionFile, O_WRITE | O_CREAT | O_APPEND)) return false;

  for (int i = 0; i < count; i++) {
    updated.println(lines[i]);
  }

  updated.close();
  return true;
}

int getSessionCount() {
  SdFile file;
  if (!file.open(sessionFile, O_READ)) return 0;

  int count = 0;
  bool skipHeader = true;
  char lineBuffer[256];

  while (file.available()) {
    file.fgets(lineBuffer, sizeof(lineBuffer));
    String line = String(lineBuffer);
    line.trim();

    if (skipHeader) {
      skipHeader = false;
      continue;
    }

    if (line.length() > 0) count++;
  }

  file.close();
  return count;
}

String readAllSessions() {
  SdFile file;
  if (!file.open(sessionFile, O_READ)) return "ERROR: File not found";

  String content = "";
  char lineBuffer[256];

  while (file.available()) {
    file.fgets(lineBuffer, sizeof(lineBuffer));
    String line = String(lineBuffer);
    line.trim();
    content += line + "\n";
  }

  file.close();
  return content;
}

void getAllSessionIds(int *idList, int &count) {
  count = 0;
  SdFile file;
  if (!file.open(sessionFile, O_READ)) return;

  bool skipHeader = true;
  char lineBuffer[256];

  while (file.available()) {
    file.fgets(lineBuffer, sizeof(lineBuffer));
    String line = String(lineBuffer);
    line.trim();

    if (skipHeader) {
      skipHeader = false;
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1) continue;

    String idStr = line.substring(0, commaIndex);
    idStr.trim();
    int id = idStr.toInt();

    if (id >= 0) {
      idList[count++] = id;
      if (count >= 128) break;
    }
  }

  file.close();
}

bool clearSessionFile() {
  if (sd.exists(sessionFile)) {
    sd.remove(sessionFile);
    SdFile file;
    if (!file.open(sessionFile, O_WRITE | O_CREAT | O_APPEND)) return false;
    file.println("id,material,diameter,humidity,temperature,dateTime,average,min,max,count,head,sample0,sample1,sample2,sample3,sample4,sample5,sample6,sample7,sample8,sample9");
    file.flush();
    file.close();
    return true;
  }
  return false;
}

int getLastSessionId() {
  SdFile file;
  if (!file.open(sessionFile, O_READ)) return -1;

  int lastId = 0;
  bool skipHeader = true;
  char lineBuffer[256];

  while (file.available()) {
    file.fgets(lineBuffer, sizeof(lineBuffer));
    String line = String(lineBuffer);
    line.trim();

    if (skipHeader) {
      skipHeader = false;
      continue;
    }

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1) continue;

    int id = line.substring(0, commaIndex).toInt();
    if (id > lastId) lastId = id;
  }

  file.close();
  return lastId;
}

float getFreeSpacePercent() {
  if (!sd.begin(SD_CS_PIN)) return -1;

  FsVolume* vol = sd.vol();
  if (!vol) return -1;

  uint32_t totalClusters = vol->clusterCount();
  uint32_t freeClusters = vol->freeClusterCount();

  if (totalClusters == 0) return 0;

  return (float)freeClusters / totalClusters * 100.0;
}