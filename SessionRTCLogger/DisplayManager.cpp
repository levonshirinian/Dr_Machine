#include "WString.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "GraphViewer.h"
#include "StorageManager.h"
#include "DisplayManager.h"
#include <TouchScreen.h>
#include <LCDWIKI_KBV.h>
#include <ArduinoJson.h>

// LCD and touch objects
LCDWIKI_KBV mylcd(ILI9486, A3, A2, A1, A0, A4);
TouchScreen ts = TouchScreen(8, A3, A2, 9, 300);

#pragma region Colors
#define BLACK 0x0000
#define BLUE 0x001F
#define WHITE 0xFFFF
#define DARKBLUE 0x0010
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define GRAY 0x8410
#pragma endregion

#pragma region Touch screen calibration
#define TS_MINX 906
#define TS_MAXX 116
#define TS_MINY 92
#define TS_MAXY 952
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#pragma endregion

#pragma region Forward declarations
void W();
void T();
void C();
void M();
void H();
void editN();
void select_M();
String getCurrentMaterial();
String showSessionInfoScreen(int targetId);
void handle_Select_Material(int x, int y);
void handle_Select_N(int x, int y);
void drawGradient(uint16_t topColor, uint16_t bottomColor);
void menu(char m);
void drawHeader(const char* title, bool showData);
void drawWeight(int x, int y, const char* unit, bool largeFont);
void displayCurrentScreen();
boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py);
void loadSessionToGraph(int id);
int circularDescendingIndex(int& idx, bool increment, int arraySize);
void MapPointRotuation(TSPoint& p);
void updateValuesOnScreen();
#pragma endregion

#pragma region Global state
const char* materials[] = { "cotton", "wool", "linen", "jute", "silk", "polyester", "Nylon" };
int currentRotation = 3;
float currentTemp = 35.0;
float currentHum = 70.0;
float currentWeight = 0.0;
bool limitSwitchState = false;
int currentM = 0;
float currentN = 1.25;
float newN = 1.25;
char currentScreen = 'T';
unsigned long lastUpdateTime = 0;
const long refreshInterval = 500;
int sessionIndex[128];
int sessionCount = 0;
int currentSessionIndex = 0;
#pragma endregion

void initializeDisplay() {
  mylcd.Init_LCD();
  mylcd.Set_Rotation(currentRotation);
  mylcd.Fill_Screen(BLACK);
  W();
  mylcd.Fill_Screen(BLACK);
  displayCurrentScreen();
  getAllSessionIds(sessionIndex, sessionCount);
  currentSessionIndex = sessionCount - 1;
}

void updateValuesOnScreen() {
  if(currentScreen == 'M') return;
  if(!limitSwitchState){  currentWeight = 0.0;}
    switch (currentScreen) {
        case 'T':
            drawWeight(100, 160, "cN", true);

            mylcd.Set_Text_Size(3);
            mylcd.Set_Text_colour(limitSwitchState ? GREEN : RED);

            mylcd.Set_Text_Back_colour(BLACK);
            mylcd.Print_String(limitSwitchState ? "LIMIT: PRESSED  " : "LIMIT: RELEASED", 100, 280);

            mylcd.Set_Text_Size(2);
            char tempBuffer[10], humBuffer[10];
            dtostrf(currentTemp, 2, 0, tempBuffer);
            dtostrf(currentHum, 3, 0, humBuffer);

            mylcd.Set_Text_Back_colour(GRAY);
            mylcd.Set_Text_colour(WHITE);
            mylcd.Print_String(tempBuffer, 340, 8);
            mylcd.Print_String(humBuffer, 420, 8);
            break;
        case 'C':
            drawWeight(100, 100, "cN", true);
            break;
        case 'H':
            drawWeight(100, 160, "G/Cm", true);
            break;
    }
}

void updateDisplaySession(){
  String material = showSessionInfoScreen(currentSessionIndex);
  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  String line = material + " " + String(currentSessionIndex) + "/" + String(sessionCount);
  mylcd.Fill_Rectangle(260, 100, 480, 125); 
  mylcd.Print_String(line.c_str(), 260, 100);
}

void updateDisplay(float temp, float hum, float weight, bool limitState) {
  currentTemp = temp;
  currentHum = hum;
  currentWeight = weight;
  limitSwitchState = limitState;

  if (millis() - lastUpdateTime > refreshInterval) {
    updateValuesOnScreen(); 
    lastUpdateTime = millis();
  }
}

String getCurrentMaterial() 
{
  return materials[currentM];
}

void handleTouch() {
  TSPoint p = ts.getPoint();
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  if (!(p.z > MINPRESSURE && p.z < MAXPRESSURE)) return;

  char oldScreen = currentScreen;
  MapPointRotuation(p);
  if( currentScreen == 'g' ) {
    if(is_pressed(430, 0, 480, 70, p.x, p.y))currentScreen = 'M';}
  else if (is_pressed(0, 0, 105, 40, p.x, p.y)) currentScreen = 'T';
  else if (is_pressed(106, 0, 212, 40, p.x, p.y)) currentScreen = 'C';
  else if (is_pressed(213, 0, 318, 40, p.x, p.y)) currentScreen = 'M';
  else if (is_pressed(370, 0, 480, 40, p.x, p.y)) currentScreen = 'H';
  else if (currentScreen == 'T' && is_pressed(90, 60, 130, 155, p.x, p.y)) currentScreen = 's';
  else if (currentScreen == 'T' && is_pressed(150, 60, 200, 155, p.x, p.y)) currentScreen = 'e';
  else if (currentScreen == 's') handle_Select_Material(p.x, p.y);
  else if (currentScreen == 'e'){ handle_Select_N(p.x, p.y);  Serial.println("handle_Select_N called");}
  else if (currentScreen == 'M') {
    if (is_pressed(230, 290, 320, 320, p.x, p.y)){ circularDescendingIndex(currentSessionIndex, true, sessionCount); updateDisplaySession(); }
    else if (is_pressed(321, 290, 410, 320, p.x, p.y)){ circularDescendingIndex(currentSessionIndex, false, sessionCount); updateDisplaySession(); }
    else if (is_pressed(445, 230, 480, 320, p.x, p.y)) deleteSessionById(currentSessionIndex);
    else if (is_pressed(450, 45, 480, 125, p.x, p.y)) currentScreen = 'g';
  }

  if (oldScreen != currentScreen) {
    mylcd.Fill_Screen(BLACK);
    displayCurrentScreen();
  }

  Serial.print("X: "); Serial.print(p.x);
  Serial.print(" Y: "); Serial.println(p.y);
  Serial.print("currentScreen: "); Serial.println(currentScreen);

}

boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py) {
  return (px > x1 && px < x2) && (py > y1 && py < y2);
}

void drawGradient(uint16_t topColor, uint16_t bottomColor) {
  mylcd.Fill_Screen(BLACK);
}

void menu(char m) {
  mylcd.Set_Draw_color(BLACK);
  mylcd.Fill_Rectangle(0, 0, 70, 320);
  uint16_t highlight = BLUE;
  uint16_t normal = GRAY;
  const char labels[4] = { 'T', 'C', 'M', 'H' };
  for (int i = 0; i < 4; i++) {
    int y1 = i * 80;
    uint16_t btnColor = (m == labels[i]) ? highlight : normal;
    mylcd.Set_Draw_color(btnColor);
    mylcd.Fill_Rectangle(0, y1, 70, y1 + 80);
    mylcd.Set_Text_Back_colour(btnColor);
    mylcd.Set_Text_colour(WHITE);
    mylcd.Set_Text_Size(6);
    mylcd.Print_String(String(labels[i]).c_str(), 20, y1 + 15);
  }
  mylcd.Set_Text_Back_colour(BLACK);
}

void drawHeader(const char* title, bool showData) {
  mylcd.Set_Draw_color(GRAY);
  mylcd.Fill_Rectangle(70, 0, 480, 40);
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_Back_colour(WHITE);
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
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(largeFont ? 12 : 5);
  char buf[10];
  dtostrf(currentWeight, 4, 0, buf);
  mylcd.Print_String(buf, x, y);
  mylcd.Set_Text_Size(4);
  mylcd.Print_String(unit, x + 250, y + 80);
}

void W() {
  mylcd.Fill_Screen(BLACK);
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(7);
  mylcd.Print_String("WELCOME", 90, 70);
  mylcd.Set_Text_Size(14);
  mylcd.Set_Text_colour(BLUE);
  mylcd.Print_String("DTX", 160, 160);
}

void T() {
  newN = currentN;
  mylcd.Fill_Screen(BLACK);
  menu('T');
  drawHeader("TENSILE TEST", true);
  mylcd.Set_Text_Size(4);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Print_String(materials[currentM], 90, 60);
  char buf[10];
  dtostrf(currentN, 1, 2, buf);
  mylcd.Print_String("N: ", 90, 100);
  mylcd.Print_String(buf, 125, 100);
  drawWeight(100, 160, "cN", true);
  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(limitSwitchState ? GREEN : RED);
  mylcd.Print_String(limitSwitchState ? "LIMIT: PRESSED" : "LIMIT: RELEASED", 100, 280);
}

void C() {
  mylcd.Fill_Screen(BLACK);
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
  mylcd.Fill_Screen(BLACK);
  String material = showSessionInfoScreen(currentSessionIndex);
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
  mylcd.Print_String("Graph", 72, 290);
}

void H() {
  mylcd.Fill_Screen(BLACK);
  menu('H');
  drawHeader("HARDNESS TEST", true);
  drawWeight(100, 160, "G/Cm", true);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Reading #3", 420, 250);
}

void editN() {
  mylcd.Fill_Screen(BLACK);
  menu('T');
  mylcd.Set_Text_colour(WHITE);
  drawHeader("Edit N Value", false);
  mylcd.Set_Text_Size(4);
  mylcd.Print_String("Edit N Value", 90, 50);
  mylcd.Print_String("Save", 390, 290);
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

void updateDisplayNewNValue(float newValue) {
  newN = newValue;
  mylcd.Set_Text_Size(8);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Draw_color(BLACK);
  mylcd.Fill_Rectangle(175, 180, 325, 230); 
  mylcd.Print_String(String(newN), 175, 180);
}

void select_M() {
  mylcd.Fill_Screen(BLACK);
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
  if (is_pressed(150, 65, 180, 175, x, y)) currentM = 0;
  else if (is_pressed(195, 65, 230, 175, x, y)) currentM = 1;
  else if (is_pressed(240, 65, 275, 175, x, y)) currentM = 2;
  else if (is_pressed(300, 65, 330, 175, x, y)) currentM = 3;
  else if (is_pressed(345, 65, 375, 175, x, y)) currentM = 4;
  else if (is_pressed(405, 65, 425, 175, x, y)) currentM = 5;
  else if (is_pressed(440, 65, 465, 175, x, y)) currentM = 6;
  currentScreen = 'T';
}

void handle_Select_N(int x, int y) {
  if (is_pressed(190, 115, 260, 140, x, y)) { newN += 1; if (newN > 10) newN -= 10; updateDisplayNewNValue(newN); }
  else if (is_pressed(390, 115, 455, 140, x, y)) { newN -= 1; if (newN < 0) newN = 0.01; updateDisplayNewNValue(newN); }
  else if (is_pressed(190, 185, 260, 210, x, y)) { newN += 0.1; if (newN > 10) newN -= 10; updateDisplayNewNValue(newN); }
  else if (is_pressed(390, 185, 455, 210, x, y)) { newN -= 0.1; if (newN < 0) newN = 0.01; updateDisplayNewNValue(newN); }
  else if (is_pressed(190, 220, 260, 245, x, y)) { newN += 0.01; if (newN > 10) newN -= 10; updateDisplayNewNValue(newN); }
  else if (is_pressed(390, 220, 455, 245, x, y)) { newN -= 0.01; if (newN < 0) newN = 0.01; updateDisplayNewNValue(newN); }
  if (is_pressed(445, 270, 480, 320, x, y)) { currentN = newN; currentScreen = 'T'; displayCurrentScreen(); }
}

void displayCurrentScreen() {
  switch (currentScreen) {
    case 'T': T(); break;
    case 'C': C(); break;
    case 'M': M(); break;
    case 'H': H(); break;
    case 'e': editN(); break;
    case 's': select_M(); break;
    case 'g': loadSessionToGraph(currentSessionIndex); break;
  }
}

int circularDescendingIndex(int& idx, bool increment, int arraySize) {
  if (increment) idx = (idx + 1) % arraySize;
  else idx = (idx - 1 + arraySize) % arraySize;
  return arraySize - 1 - idx;
}

void MapPointRotuation(TSPoint& p) {
  long rawX = p.x;
  long rawY = p.y;
  long mappedLong = map(rawX, TS_MINX, TS_MAXX, 0, 480);
  long mappedShort = map(rawY, TS_MINY, TS_MAXY, 0, 320);
  long pixelX = 0, pixelY = 0;
  switch (currentRotation) {
    case 1: pixelX = mappedLong; pixelY = 320 - mappedShort; break;
    case 3: pixelX = 480 - mappedLong; pixelY = mappedShort; break;
    default: pixelX = mappedLong; pixelY = mappedShort; break;
  }
  p.x = pixelX; p.y = pixelY;
}

void loadSessionToGraph(int id) {
  GraphSession gSession;
  String line = getSessionById(id);
  if (line == "NOT FOUND") return;
  gSession.reset();
  int fieldIndex = 0, lastIndex = 0;
  String fields[25];
  for (int j = 0; j < line.length(); j++) {
    if (line[j] == ',' || j == line.length() - 1) {
      int endIndex = (j == line.length() - 1) ? j + 1 : j;
      fields[fieldIndex++] = line.substring(lastIndex, endIndex);
      lastIndex = j + 1;
    }
  }
  for (int i = 0; i < 10; i++) {
    float val = fields[11 + i].toFloat();
    gSession.addPoint(val, i);
  }
  drawGraph(gSession);
}

String showSessionInfoScreen(int targetId) {
  const int contentX = 75;
  const int contentY = 160;
  const int contentW = 320;
  const int contentH = 300;
  const int lineHeight = 25;

  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Draw_color(BLACK);
  mylcd.Fill_Rectangle(70, contentY, contentW + 60, contentH -15);
  mylcd.Set_Draw_color(GRAY);

  String line = getSessionById(targetId);
  if (line == "NOT FOUND") {
    mylcd.Print_String("Session not found", contentX, contentY + 10);
    return "";
  }

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

  String Id = fields[0];
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
  y += lineHeight;

  String line5 = "Id:" + Id;
  mylcd.Print_String(line5.c_str(), contentX, y);

  return material;
}
