#include "WString.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "StorageManager.h"
#include "DisplayManager.h"
#include <TouchScreen.h>
#include <LCDWIKI_GUI.h>
#include <LCDWIKI_KBV.h>
#include <ArduinoJson.h>

// LCD and touch objects
LCDWIKI_KBV mylcd(ILI9486, A3, A2, A1, A0, A4);
TouchScreen ts = TouchScreen(8, A3, A2, 9, 300);

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

// Touch screen calibration
#define TS_MINX 906
#define TS_MAXX 116
#define TS_MINY 92
#define TS_MAXY 952
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// Forward declarations for internal display functions
void W();
void T();
void C();
void M();
void H();
void editN();
void select_M();
void showFileListScreen();
void handle_Select_Material(int x, int y);
void handle_Select_N(int x, int y);
void drawGradient(uint16_t topColor, uint16_t bottomColor);
// void handle_SaveN(int x, int y);
void menu(char m);
void drawHeader(const char* title, bool showData);
void drawWeight(int x, int y, const char* unit, bool largeFont);
void displayCurrentScreen();
boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py);

const char* materials[] = { "cotton", "wool", "linen", "jute", "silk", "polyester", "Nylon" };

// Global state
float currentTemp = 35.0;
float currentHum = 70.0;
float currentWeight = 0.0;
bool limitSwitchState = false;
int currentM = 0;
float currentN = 9.25;
float newN = 9.25;
char currentScreen = 'T';
unsigned long lastUpdateTime = 0;
const long refreshInterval = 500;


// initializeDisplay
void initializeDisplay() {
  mylcd.Init_LCD();
  mylcd.Set_Rotation(3);
  mylcd.Fill_Screen(BLACK);
  W();
  // delay(2000);
  currentScreen = 'M';
  mylcd.Fill_Screen(BLACK);
  displayCurrentScreen();
}

// updateDisplay
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

// handleTouch
void handleTouch() {
  // refresh only when the screen changed
  char oldScreen = currentScreen;

  TSPoint p = ts.getPoint();
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = mylcd.Get_Display_Width() - map(p.x, TS_MINX, TS_MAXX, 0, mylcd.Get_Display_Width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, mylcd.Get_Display_Height());

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

    // Serial.print("x: ");
    // Serial.print(p.x);
    // Serial.print(" y: ");
    // Serial.println(p.y);


    if (oldScreen != currentScreen) {
      mylcd.Fill_Screen(BLACK);
      displayCurrentScreen();
    }
  }
}

// باقي الدوال: drawGradient, menu, drawHeader, drawWeight, T, C, M, H, editN, select_M, displayCurrentScreen, is_pressed
// تنقل كما هي من الكود الأصلي

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

  float samples[] = { 1, 2, 5 };
  // if (logSessionData(
  //       "s3.csv",  // ← تأكد أن الامتداد صحيح
  //       "cotton",
  //       1.25,
  //       32,
  //       23,
  //       "2025/10/23",
  //       3.6,
  //       2,
  //       5,
  //       3,
  //       1,
  //       samples,  // ← مرر اسم المصفوفة
  //       3         // ← عدد العينات الفعلي
  //       )) {
  //   Serial.println("Add to file successfully");
  // }

  drawGradient(GREEN, BLACK);
  menu('M');
  drawHeader("MEMORY", true);
  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Print_String("Data Slots: 100% Free", 100, 60);
  mylcd.Print_String("Material:", 100, 100);
  mylcd.Print_String(materials[currentM], 260, 100);

  mylcd.Set_Text_Size(8);
  mylcd.Print_String("+", 435, 150);
  mylcd.Print_String("-", 435, 210);
  // mylcd.Draw_Triangle(435,150, 465,150,430,120);

  mylcd.Set_Text_Size(4);
  mylcd.Print_String("Delete", 335, 290);

  //display list
  showFileListScreen();
}

void showFileListScreen() {
  String files = listFiles();
  int y = 160;
  mylcd.Set_Text_Size(4);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Fill_Rectangle(70,y,320,300);

  int lineStart = 0;
  for (int i = 0; i < files.length(); i++) {
    if (files[i] == '\n') {
      String line = files.substring(lineStart, i);
      mylcd.Print_String(line.c_str(), 80, y);
      Serial.println(line.c_str());
      y += 35;
      lineStart = i + 1;
    }
  }
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
