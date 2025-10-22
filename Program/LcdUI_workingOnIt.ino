#include <TouchScreen.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
// Corrected constructor for the LCD object
TFT_eSPI tft = TFT_eSPI();

// Define touch screen pins and calibration
#define XP 19  // was A3 6 21
#define YP 15 // was A2 55 15
#define XM 33  // was A1 56 33
#define YM 13  // was A0 (or 9) 7 13
// قيم المعايرة
#define TS_MINX 906
#define TS_MAXX 116
#define TS_MINY 92
#define TS_MAXY 952
#define MINPRESSURE 10
#define MAXPRESSURE 1000
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
const char* materials[] = {
  "cotton",
  "wool",
  "linen",
  "jute",
  "silk",
  "polyester",
  "Nylon"
};
// Define colors and display constants
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Global variables for sensor data
float currentTemp = 35.0;
float currentHum = 70.0;
float currentWeight = 0.0;
int currentM = 0;
bool limitSwitchState = false;
float currentN = 1.25;
char currentScreen = 'W';

// *إضافة جديدة:* متغير عام لتخزين دوران الشاشة (0, 1, 2, 3)
uint8_t currentRotation = 1;  // (portrait)

// Variables for managing display refresh rate
unsigned long lastUpdateTime = 0;
const long refreshInterval = 500;  // Refresh every 500 milliseconds

// --- Function Prototypes ---
void menu(char m);
void drawHeader(const char* title, bool showData = true);
void drawWeight(int x, int y, const char* unit, bool largeFont = true);
void T();
void C();
void M();
void H();
void W();
void editN();
void select_M();
void displayCurrentScreen();
boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py);
void MapPointRotuation(TSPoint& p);  // تم تحديث النمط لقبول TSPoint بالمرجع

void setup() {
  Serial.begin(115200);  // Use hardware serial for communication with ESP32
  tft.init();
  //Serial.println("1");
  // *تعديل:* استخدام المتغير العام currentRotation لضبط الدوران
  tft.setRotation(currentRotation);
  tft.fillScreen(TFT_BLACK);
  W();  // Show welcome screen
  //Serial.println("2");
  delay(3000);
  //Serial.println("3");
  currentScreen = 'H';
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  // Check for incoming data from ESP32
  // Serial.println("4");

  //TSPoint p = ts.getPoint();
  // delay(1);

  // pinMode(XM, OUTPUT);
  // pinMode(YP, OUTPUT);

  // Serial.print("  pixel_x:");
  //   Serial.print(p.x);
  //   Serial.print(", pixel_y:");
  //   Serial.print(p.y);
  // delay(2000);

  // tft.fillScreen(TFT_BLACK);
  // // delay(2000);
  // H();

  // tft.fillScreen(TFT_BLACK);

  // Handle touch input
  TSPoint p = ts.getPoint();

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {

    // معايرة وتحويل إحداثيات اللمس بناءً على الدوران الحالي
    MapPointRotuation(p);
    // الآن p.x و p.y تحتويان على إحداثيات الشاشة الفعلية بعد المعايرة والدوران

    if (p.z > ts.pressureThreshhold) {
      // *تعديل:* عرض إحداثيات الشاشة المعايرة (p.x, p.y) بدلاً من الإحداثيات الخام
      Serial.print("X = ");
      Serial.print(p.x);
      Serial.print("\tY = ");
      Serial.print(p.y);
      Serial.print("\tPressure = ");
      Serial.println(p.z);
    }
    // Main navigation buttons
    if (is_pressed(0, 0, 120, 60, p.x, p.y)) {
      currentScreen = 'T';
      tft.fillScreen(TFT_BLACK);
    } else if (is_pressed(120, 0, 240, 60, p.x, p.y)) {
      currentScreen = 'C';
      tft.fillScreen(TFT_BLACK);
    } else if (is_pressed(240, 0, 360, 60, p.x, p.y)) {
      currentScreen = 'M';
      tft.fillScreen(TFT_BLACK);
    } else if (is_pressed(360, 0, 480, 60, p.x, p.y)) {
      currentScreen = 'H';
      tft.fillScreen(TFT_BLACK);
    }

    // Handle specific screen buttons
    switch (currentScreen) {
      case 'T':
        if (is_pressed(80, 60, 200, 120, p.x, p.y)) {
          currentScreen = 's';
          tft.fillScreen(TFT_BLACK);
        } else if (is_pressed(80, 130, 200, 190, p.x, p.y)) {
          currentScreen = 'e';
          tft.fillScreen(TFT_BLACK);
        }
        break;
      case 'C':
        if (is_pressed(120, 260, 240, 320, p.x, p.y)) {
          // Save functionality (not implemented in original code)
        } else if (is_pressed(410, 90, 480, 150, p.x, p.y)) {
          // + button
        } else if (is_pressed(410, 200, 480, 260, p.x, p.y)) {
          // - button
        }
        break;
      case 'M':
        if (is_pressed(190, 280, 310, 320, p.x, p.y)) {
          // Delete functionality (not implemented in original code)
        }
        break;
      case 's':  // Select material
        if (is_pressed(0, 60, 60, 320, p.x, p.y)) {
          currentScreen = 'T';
          tft.fillScreen(TFT_BLACK);
        } else {
          for (int i = 0; i < 7; i++) {
            if (is_pressed(80, 50 + (i * 40), 400, 50 + (i * 40) + 30, p.x, p.y)) {
              currentM = i;
              currentScreen = 'T';
              tft.fillScreen(TFT_BLACK);
              break;
            }
          }
        }
        break;
      case 'e':  // Edit N
        if (is_pressed(190, 280, 310, 320, p.x, p.y)) {
          currentScreen = 'T';
          tft.fillScreen(TFT_BLACK);
        } else if (is_pressed(120, 0, 180, 60, p.x, p.y)) {
          currentN += 1.0;
        } else if (is_pressed(120, 200, 180, 260, p.x, p.y)) {
          currentN -= 1.0;
        } else if (is_pressed(240, 0, 300, 60, p.x, p.y)) {
          currentN += 0.1;
        } else if (is_pressed(240, 200, 300, 260, p.x, p.y)) {
          currentN -= 0.1;
        } else if (is_pressed(360, 0, 420, 60, p.x, p.y)) {
          currentN += 0.01;
        } else if (is_pressed(360, 200, 420, 260, p.x, p.y)) {
          currentN -= 0.01;
        }
        // ضمان عدم نزول القيمة عن الصفر
        if (currentN < 0) currentN = 0.0;
        break;
    }
  }

  //Update display based on refresh interval and screen type
  if (millis() - lastUpdateTime > refreshInterval) {
    displayCurrentScreen();
    lastUpdateTime = millis();
  }
}

// --- Display Management and UI Functions ---

boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py) {
  return (px > x1 && px < x2) && (py > y1 && py < y2);
}

void menu(char m) {
  tft.drawLine(72, 0, 72, 320, TFT_WHITE);

  switch (m) {
    case 'T':
      tft.drawRect(0, 0, 70, 80, TFT_WHITE);
      tft.drawRect(1, 1, 69, 79, TFT_WHITE);
      break;
    case 'C':
      tft.drawRect(0, 80, 70, 80, TFT_WHITE);
      tft.drawRect(1, 81, 69, 80, TFT_WHITE);
      break;
    case 'M':
      tft.drawRect(0, 160, 70, 80, TFT_WHITE);
      tft.drawRect(1, 161, 69, 80, TFT_WHITE);
      break;
    case 'H':
      tft.drawRect(0, 240, 70, 320, TFT_WHITE);
      tft.drawRect(1, 241, 69, 319, TFT_WHITE);
      break;
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(7);
  tft.drawString("T", 18, 12);
  tft.drawString("C", 18, 92);
  tft.drawString("M", 18, 172);
  tft.drawString("H", 18, 252);
}

void drawHeader(const char* title, bool showData) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(5);
  tft.drawString(title, 80, 0);
  tft.drawLine(72, 40, 480, 40, TFT_WHITE);
  if (showData) {
    tft.setTextSize(3);
    char tempBuffer[10], humBuffer[10];
    dtostrf(currentTemp, 2, 0, tempBuffer);
    dtostrf(currentHum, 3, 0, humBuffer);
    tft.drawString("T:", 280, 50);
    tft.drawString(tempBuffer, 300, 50);
    tft.drawString(" H:", 350, 50);
    tft.drawString(humBuffer, 370, 50);
    tft.drawString("%", 450, 50);
  }
}

void drawWeight(int x, int y, const char* unit, bool largeFont) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(largeFont ? 12 : 5);
  char weightBuffer[10];
  dtostrf(currentWeight, 4, 0, weightBuffer);
  tft.drawString(weightBuffer, x, y);
  tft.setTextSize(4);
  tft.drawString(unit, x + 250, y + 80);
}

void W() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(7);
  tft.drawString("welcome", 90, 50);
  tft.setTextSize(16);
  tft.drawString("DTX", 90, 120);
}

void T() {
  menu('T');
  drawHeader("TENSILE test");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(5);
  tft.drawString(materials[currentM], 80, 0);

  tft.setTextSize(3);
  char currentNBuffer[10];
  dtostrf(currentN, 1, 2, currentNBuffer);
  tft.drawString(currentNBuffer, 80, 50);
  tft.drawString("mm", 150, 50);

  tft.drawString("No :0", 280, 80);
  tft.drawString("Damp:9", 80, 80);

  tft.setTextSize(12);
  char weightBuffer[10];
  dtostrf(currentWeight, 4, 0, weightBuffer);
  tft.drawString(weightBuffer, 100, 160);

  tft.setTextSize(4);
  tft.drawString("cN", 420, 240);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (limitSwitchState) {
    tft.drawString("Limit: PRESSED", 100, 280);
  } else {
    tft.drawString("Limit: RELEASED", 100, 280);
  }
}

void C() {
  menu('C');
  drawHeader("Cal.Adjustment", false);
  drawWeight(100, 100, "cN", true);
  tft.setTextSize(6);
  tft.drawString("SAVE", 120, 260);
  tft.setTextSize(11);
  tft.drawString("+", 410, 90);
  tft.drawString("-", 410, 200);
}

void M() {
  menu('M');
  drawHeader("MOMERY");
  tft.setTextSize(5);
  tft.drawString("No:009", 280, 0);

  tft.setTextSize(3);
  tft.drawString("100%free", 300, 50);
  tft.drawString("jute", 80, 80);
  tft.drawString("1.6mm", 80, 50);
  tft.drawString("T:", 80, 110);
  tft.drawString("delete", 190, 280);

  char tempBuffer[10], humBuffer[10];
  dtostrf(currentTemp, 2, 0, tempBuffer);
  dtostrf(currentHum, 3, 0, humBuffer);
  tft.drawString(tempBuffer, 100, 110);
  tft.drawString(" H:", 150, 110);
  tft.drawString(humBuffer, 170, 110);
  tft.drawString("%", 240, 110);
  tft.drawString("MIN: 854cn", 80, 140);
  tft.drawString("MAX: 1658cn", 80, 170);
  tft.drawString("AV:1275cn", 80, 200);
}

void H() {
  menu('H');
  drawHeader("HARDNESS test");
  drawWeight(100, 160, "G/Cm", true);
  tft.setTextSize(3);
  tft.drawString("3", 420, 250);
}

void editN() {
  menu('T');
  tft.setTextSize(5);
  tft.drawString("Edit N Value", 80, 0);
  tft.drawLine(72, 40, 480, 40, TFT_WHITE);
  tft.setTextSize(12);
  tft.drawString("1.59", 120, 100);
  tft.setTextSize(5);
  char currentNBuffer[10];
  dtostrf(currentN, 1, 2, currentNBuffer);
  tft.drawString(currentNBuffer, 420, 150);
  tft.drawString("mm", 420, 150);
  tft.drawString("SAVE", 190, 280);
  tft.setTextSize(10);
  tft.drawString("+", 120, 0);
  tft.drawString("-", 120, 200);
  tft.drawString("+", 240, 0);
  tft.drawString("-", 240, 200);
  tft.drawString("+", 360, 0);
  tft.drawString("-", 360, 200);
}

void select_M() {
  menu('T');
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(5);
  tft.drawString("Select material", 80, 0);
  tft.setTextSize(3);
  tft.drawLine(72, 40, 480, 40, TFT_WHITE);
  for (int i = 0; i < 7; i++) {
    tft.drawString(materials[i], 80, 50 + (i * 40));
    tft.drawFastHLine(80, 50 + (i * 40) - 10, 330, TFT_WHITE);
  }
}

void displayCurrentScreen() {
  switch (currentScreen) {
    case 'T':
      T();
      break;
    case 'C':
      C();
      break;
    case 'M':
      M();
      break;
    case 'H':
      H();
      break;
    case 'e':
      editN();
      break;
    case 's':
      select_M();
      break;
  }
}

// 🎯 التعديل الرئيسي: دالة معايرة وتدوير نقطة اللمس
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