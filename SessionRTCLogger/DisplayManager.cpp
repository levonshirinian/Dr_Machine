#include "WString.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "StorageManager.h"
#include "DisplayManager.h"
#include <TouchScreen.h>
// #include <LCDWIKI_GUI.h>
#include <LCDWIKI_KBV.h>
#include <ArduinoJson.h>

// LCD and touch objects
LCDWIKI_KBV mylcd(ILI9486, A3, A2, A1, A0, A4);
TouchScreen ts = TouchScreen(8, A3, A2, 9, 300);

#pragma region Colors
// Colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GRAY 0x8410
#define DARKBLUE 0x0010
#define DARKGRAY 0x4208
#pragma endregion

#pragma region Touch screen calibration
// Touch screen calibration
#define TS_MINX 906
#define TS_MAXX 116
#define TS_MINY 92
#define TS_MAXY 952
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#pragma endregion

#pragma region Forward declarations for internal display functions
// Forward declarations for internal display functions
void W();
void T();
void C();
void M();
void H();
void editN();
void select_M();
String showFileListScreen(int targetId);
void handle_Select_Material(int x, int y);
void handle_Select_N(int x, int y);
void drawGradient(uint16_t topColor, uint16_t bottomColor);
// void handle_SaveN(int x, int y);
void menu(char m);
void drawHeader(const char* title, bool showData);
void drawWeight(int x, int y, const char* unit, bool largeFont);
void displayCurrentScreen();
boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py);
void MapPointRotuation(TSPoint& p);  // تم تحديث النمط لقبول TSPoint بالمرجع
#pragma endregion

#pragma region Global state
// Global state
const char* materials[] = { "cotton", "wool", "linen", "jute", "silk", "polyester", "Nylon" };
int currentRotation = 2;
float currentTemp = 35.0;
float currentHum = 70.0;
float currentWeight = 0.0;
bool limitSwitchState = false;
int currentM = 0;
float currentN = 9.25;
float newN = 9.25;
char currentScreen = 'M';
unsigned long lastUpdateTime = 0;
const long refreshInterval = 500;

#pragma region Read Session
int sessionIndex[0];
int sessionCount = 0;
int currentSessionIndex = 0;
#pragma endregion

#pragma endregion


// initializeDisplay
void initializeDisplay() {
  mylcd.Init_LCD();
  mylcd.Set_Rotation(currentRotation);
  mylcd.Fill_Screen(BLACK);
  W();
  // delay(2000);
  mylcd.Fill_Screen(BLACK);
  displayCurrentScreen();
}

void updateDisplay(float temp, float hum, float weight, bool limitState) {
  currentTemp = temp;
  currentHum = hum;
  currentWeight = weight;
  limitSwitchState = limitState;

  if (millis() - lastUpdateTime > refreshInterval) {
    displayCurrentScreen();
    lastUpdateTime = millis();
  }
}

void handleTouch() {

  TSPoint p = ts.getPoint();
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  if (!(p.z > MINPRESSURE && p.z < MAXPRESSURE)) return;


  char oldScreen = currentScreen;
  MapPointRotuation(p);
  if (is_pressed(0, 0, 105, 40, p.x, p.y)) {
    currentScreen = 'T';
  } else if (is_pressed(106, 0, 212, 40, p.x, p.y)) {
    currentScreen = 'C';
  } else if (is_pressed(213, 0, 318, 40, p.x, p.y)) {
    currentScreen = 'M';
  } else if (is_pressed(370, 0, 480, 40, p.x, p.y)) {
    currentScreen = 'H';
  } else if (currentScreen == 'T' && is_pressed(90, 60, 130, 155, p.x, p.y)) {
    currentScreen = 's';
  } else if (currentScreen == 'T' && is_pressed(150, 60, 200, 155, p.x, p.y)) {
    currentScreen = 'e';
  } else if (currentScreen == 's') {
    handle_Select_Material(p.x, p.y);
  } else if (currentScreen == 'e') {
    handle_Select_N(p.x, p.y);
  }

  Serial.print("x: ");
  Serial.print(p.x);
  Serial.print(" y: ");
  Serial.println(p.y);


  if (oldScreen != currentScreen) {
    mylcd.Fill_Screen(BLACK);
    displayCurrentScreen();
  }
}

boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py) {
  return (px > x1 && px < x2) && (py > y1 && py < y2);
}

// Gradient Background
void drawGradient(uint16_t topColor, uint16_t bottomColor) {
  for (int y = 0; y < mylcd.Get_Display_Height(); y++) {
    uint8_t r = ((topColor >> 11) & 0x1F) + (((bottomColor >> 11) & 0x1F) - ((topColor >> 11) & 0x1F)) * y / mylcd.Get_Display_Height();
    uint8_t g = ((topColor >> 5) & 0x3F) + (((bottomColor >> 5) & 0x3F) - ((topColor >> 5) & 0x3F)) * y / mylcd.Get_Display_Height();
    uint8_t b = (topColor & 0x1F) + ((bottomColor & 0x1F) - (topColor & 0x1F)) * y / mylcd.Get_Display_Height();
    uint16_t color = (r << 11) | (g << 5) | b;
    mylcd.Set_Draw_color(color);
    mylcd.Draw_Fast_HLine(0, y, mylcd.Get_Display_Width());
  }
}

void menu(char m) {
  // Sidebar background
  mylcd.Set_Draw_color(DARKBLUE);
  mylcd.Fill_Rectangle(0, 0, 70, 320);

  // Active button highlight
  uint16_t highlight = ORANGE;
  uint16_t normal = BLUE;

  for (int i = 0; i < 4; i++) {
    int y1 = i * 80;
    mylcd.Set_Draw_color((m == "TCMH"[i]) ? highlight : normal);
    mylcd.Fill_Rectangle(0, y1, 70, y1 + 80);
  }

  // Text labels
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(6);
  mylcd.Print_String("T", 20, 15);
  mylcd.Print_String("C", 20, 95);
  mylcd.Print_String("M", 20, 175);
  mylcd.Print_String("H", 20, 255);
}

void drawHeader(const char* title, bool showData) {
  mylcd.Set_Draw_color(GRAY);
  mylcd.Fill_Rectangle(70, 0, 480, 40);
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_Back_colour(GRAY);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String(title, 80, 8);

  if (showData) {
    mylcd.Set_Text_Size(2);
    char tempBuffer[10], humBuffer[10];
    dtostrf(currentTemp, 2, 0, tempBuffer);
    dtostrf(currentHum, 3, 0, humBuffer);
    mylcd.Print_String("T:", 320, 8);
    mylcd.Print_String(tempBuffer, 340, 8);
    mylcd.Print_String("C H:", 380, 8);
    mylcd.Print_String(humBuffer, 420, 8);
    mylcd.Print_String("%", 460, 8);
  }
}

void drawWeight(int x, int y, const char* unit, bool largeFont) {
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_colour(YELLOW);
  mylcd.Set_Text_Size(largeFont ? 12 : 5);
  char buf[10];
  dtostrf(currentWeight, 4, 0, buf);
  mylcd.Print_String(buf, x, y);
  mylcd.Set_Text_Size(4);
  mylcd.Print_String(unit, x + 250, y + 80);
}

void W() {
  drawGradient(DARKBLUE, BLACK);
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(7);
  mylcd.Print_String("WELCOME", 90, 70);
  mylcd.Set_Text_Size(14);
  mylcd.Set_Text_colour(ORANGE);
  mylcd.Print_String("DTX", 160, 160);
}

void T() {
  newN = currentN;
  drawGradient(BLUE, BLACK);
  menu('T');
  drawHeader("TENSILE TEST", true);
  mylcd.Set_Text_Size(4);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Print_String(materials[currentM], 90, 60);

  char buf[10];
  dtostrf(currentN, 1, 2, buf);
  mylcd.Print_String("N:", 90, 100);
  mylcd.Print_String(buf, 120, 100);
  drawWeight(100, 160, "cN", true);

  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(limitSwitchState ? GREEN : RED);
  mylcd.Print_String(limitSwitchState ? "LIMIT: PRESSED" : "LIMIT: RELEASED", 100, 280);
}

void C() {
  drawGradient(MAGENTA, BLACK);
  menu('C');
  drawHeader("CALIBRATION", false);
  drawWeight(100, 100, "cN", true);
  mylcd.Set_Text_Size(6);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Print_String("SAVE", 120, 260);

  mylcd.Set_Text_Size(8);
  mylcd.Print_String("+", 435, 150);
  mylcd.Print_String("-", 435, 210);
}

void M() {
  drawGradient(GREEN, BLACK);

  getAllSessionIds(sessionIndex, sessionCount);

  currentSessionIndex = sessionCount - 1;
  String material = showFileListScreen(sessionIndex);

  menu('M');
  drawHeader("MEMORY", true);
  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Print_String("Data Slots: 100% Free", 100, 60);
  mylcd.Print_String("Material:", 100, 100);

  String line = material + " " + String(currentSessionIndex) + "/" + String(sessionCount);
  mylcd.Print_String(line.c_str(), 260, 100);

  mylcd.Set_Text_Size(8);
  mylcd.Print_String("+", 435, 150);
  mylcd.Print_String("-", 435, 210);

  mylcd.Set_Text_Size(4);
  mylcd.Print_String("Delete", 335, 290);
}

String showSessionInfoScreen(int targetId) {
  const int contentX = 75;
  const int contentY = 160;
  const int contentW = 320;
  const int contentH = 300;
  const int lineHeight = 25;

  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Fill_Rectangle(70, contentY, contentW, contentH);

  String line = getSessionById(targetId);
  if (line == "NOT FOUND") {
    mylcd.Print_String("Session not found", contentX, contentY + 10);
    return "";
  }

  // تقسيم السطر إلى الحقول المطلوبة
  String fields[15];
  int fieldIndex = 0;
  int lastIndex = 0;

  for (int j = 0; j < line.length(); j++) {
    if (line[j] == ',' || j == line.length() - 1) {
      int endIndex = (j == line.length() - 1) ? j + 1 : j;
      fields[fieldIndex++] = line.substring(lastIndex, endIndex);
      lastIndex = j + 1;
      if (fieldIndex >= 12) break;
    }
  }

  String material = fields[1];
  String dateTime = fields[5];
  String diameter = fields[2];
  String temperature = fields[4];
  String humidity = fields[3];
  String minVal = fields[7];
  String maxVal = fields[8];
  String average = fields[6];

  int y = contentY;
  mylcd.Print_String(dateTime.c_str(), contentX, y);
  y += lineHeight;

  String line2 = "T:" + temperature + " H:" + humidity;
  mylcd.Print_String(line2.c_str(), contentX, y);
  y += lineHeight;

  String line3 = "D:" + diameter + " Avg:" + average;
  mylcd.Print_String(line3.c_str(), contentX, y);
  y += lineHeight;

  String line4 = "Min:" + minVal + " Max:" + maxVal;
  mylcd.Print_String(line4.c_str(), contentX, y);

  return material;
}


int circularDescendingIndex(int& idx, bool increment, int arraySize) {
  if (increment) {
    idx = (idx + 1) % arraySize;
  } else {
    idx = (idx - 1 + arraySize) % arraySize;
  }
  return arraySize - 1 - idx;
}

void H() {
  drawGradient(CYAN, BLACK);
  menu('H');
  drawHeader("HARDNESS TEST", true);
  drawWeight(100, 160, "G/Cm", true);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Reading #3", 420, 250);
}

void editN() {
  drawGradient(RED, BLACK);
  menu('T');
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(4);
  mylcd.Print_String("Edit N Value", 90, 50);

  // mylcd.Print_String("Save", 370, 200);
  mylcd.Print_String("Save", 390, 290);

  // 260
  mylcd.Set_Text_Size(8);
  mylcd.Print_String(String(newN), 175, 180);
  mylcd.Set_Text_Size(6);

  mylcd.Print_String("+", 175, 125);
  mylcd.Print_String("-", 175, 250);

  mylcd.Print_String("+", 275, 125);
  mylcd.Print_String("-", 275, 250);

  mylcd.Print_String("+", 325, 125);
  mylcd.Print_String("-", 325, 250);
}

void handle_Select_N(int x, int y) {
  if (!(millis() - lastUpdateTime > refreshInterval)) {
    return;
  }

  lastUpdateTime = millis();

  if (is_pressed(190, 115, 260, 140, x, y)) {
    newN += 1;
    if (newN > 10) newN -= 10;
    displayCurrentScreen();
  } else if (is_pressed(390, 115, 455, 140, x, y)) {
    newN -= 1;
    if (newN < 0) newN = 0.01;
    displayCurrentScreen();
  } else if (is_pressed(190, 185, 260, 210, x, y)) {
    newN += 0.1;
    if (newN > 10) newN -= 10;
    displayCurrentScreen();
  } else if (is_pressed(390, 185, 455, 210, x, y)) {
    newN -= 0.1;
    if (newN < 0) newN = 0.01;
    displayCurrentScreen();
  } else if (is_pressed(190, 220, 260, 245, x, y)) {
    newN += 0.01;
    if (newN > 10) newN -= 10;
    displayCurrentScreen();
  } else if (is_pressed(390, 220, 455, 245, x, y)) {
    newN -= 0.01;
    if (newN < 0) newN = 0.01;
    displayCurrentScreen();
  }


  if (newN > 10) newN -= 10;


  if (is_pressed(445, 270, 480, 320, x, y)) {
    currentN = newN;
    currentScreen = 'T';
  }
}

void select_M() {
  drawGradient(DARKGRAY, BLACK);
  menu('T');
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(4);
  mylcd.Print_String("Select Material", 100, 50);
  mylcd.Set_Text_Size(3);
  for (int i = 0; i < 7; i++) {
    mylcd.Print_String(materials[i], 100, 100 + i * 30);
  }
}

void handle_Select_Material(int x, int y) {

  // x1 = 150 ,y1 = 65 , x2 = 460 , y2 = 175
  if (is_pressed(150, 65, 180, 175, x, y)) {
    currentM = 0;
    currentScreen = 'T';
  } else if (is_pressed(195, 65, 230, 175, x, y)) {
    currentM = 1;
    currentScreen = 'T';
  } else if (is_pressed(240, 65, 275, 175, x, y)) {
    currentM = 2;
    currentScreen = 'T';
  } else if (is_pressed(300, 65, 330, 175, x, y)) {
    currentM = 3;
    currentScreen = 'T';
  } else if (is_pressed(345, 65, 375, 175, x, y)) {
    currentM = 4;
    currentScreen = 'T';
  } else if (is_pressed(405, 65, 425, 175, x, y)) {
    currentM = 5;
    currentScreen = 'T';
  } else if (is_pressed(440, 65, 465, 175, x, y)) {
    currentM = 6;
    currentScreen = 'T';
  }
}

void displayCurrentScreen() {
  switch (currentScreen) {
    case 'T': T(); break;
    case 'C': C(); break;
    case 'M': M(); break;
    case 'H': H(); break;
    case 'e': editN(); break;
    case 's':
      select_M();
      break;
  }
}


void MapPointRotuation(TSPoint& p) {
  // قيم اللمس الخام (لا تتغير حسب الدوران)
  long rawX = p.x;
  long rawY = p.y;

  // معايرة المحاور الخام إلى النطاق البكسلي الثابت للشاشة (320x480)
  // نفترض: X الخام (الطويل) -> 480، Y الخام (القصير) -> 320

  // 1. معايرة X الخام إلى إحداثي الشاشة (0-480)
  // TS_MINX/MAXX هي أبعاد اللمس للمحور الطويل
  long mappedLong = map(rawX, TS_MINX, TS_MAXX, 0, 480);

  // 2. معايرة Y الخام إلى إحداثي الشاشة (0-320)
  // TS_MINY/MAXY هي أبعاد اللمس للمحور القصير
  long mappedShort = map(rawY, TS_MINY, TS_MAXY, 0, 320);

  // المتغيرات لتخزين إحداثيات البكسل النهائية بعد الدوران
  long pixelX = 0;
  long pixelY = 0;

  // 3. تطبيق منطق الدوران بناءً على currentRotation
  switch (currentRotation) {
    case 0:  // 0 degrees (شاشة 320x480 عمودية)
      mappedLong = map(rawX, TS_MINX, TS_MAXX, 0, 320);
      mappedShort = map(rawY, TS_MINY, TS_MAXY, 0, 480);
      // X الشاشة = Y المعايرة (القصير)
      pixelX = 480 - mappedShort;
      // Y الشاشة = X المعايرة (الطويل) معكوس
      pixelY = 320 - mappedLong;
      break;

    case 1:  // 90 degrees (شاشة 480x320 أفقية) - الحالة الافتراضية في الكود
      // X الشاشة = X المعايرة (الطويل)
      pixelX = mappedLong;
      // Y الشاشة = Y المعايرة (القصير) معكوس
      pixelY = 320 - mappedShort;
      break;

    case 2:  // 180 degrees (شاشة 320x480 عمودية)
      mappedLong = map(rawX, TS_MINX, TS_MAXX, 0, 320);
      mappedShort = map(rawY, TS_MINY, TS_MAXY, 0, 480);
      // X الشاشة = Y المعايرة (القصير) معكوس
      pixelX = mappedShort;
      // Y الشاشة = X المعايرة (الطويل)
      pixelY = mappedLong;
      break;

    case 3:  // 270 degrees (شاشة 480x320 أفقية)
      // X الشاشة = X المعايرة (الطويل) معكوس - (0,0) يصبح (480, 320)
      pixelX = 480 - mappedLong;
      // Y الشاشة = Y المعايرة (القصير)
      pixelY = mappedShort;
      break;

    default:
      // استخدام القيم المعايرة في حال عدم معرفة الدوران (0,0 في الأعلى اليسار الافتراضي)
      pixelX = mappedLong;
      pixelY = mappedShort;
      break;
  }

  // تحديث إحداثيات النقطة بالإحداثيات النهائية
  p.x = pixelX;
  p.y = pixelY;
}
