#include "GraphViewer.h"
#include <math.h>

#define GRAPH_X 50
#define GRAPH_Y 40
#define GRAPH_W 400
#define GRAPH_H 220

#define AXIS_COLOR TFT_WHITE
#define GRID_COLOR TFT_DARKGREY
#define DATA_COLOR TFT_CYAN
#define BG_COLOR TFT_BLACK
#define TEXT_COLOR TFT_WHITE

MCUFRIEND_kbv tft;

float globalMaxY = 100;
GraphSession graphSession;

void GraphSession::addPoint(float val, float seconds) {
  if (count < MAX_POINTS) {
    dataPoints[count].value = val;
    dataPoints[count].timestamp = seconds * 1000;
    count++;
  }
}

void GraphSession::reset() {
  count = 0;
}

void drawAxes(float maxY) {
  tft.fillRect(GRAPH_X - 2, GRAPH_Y - 2, GRAPH_W + 4, GRAPH_H + 4, BG_COLOR);

  tft.drawLine(GRAPH_X, GRAPH_Y + GRAPH_H, GRAPH_X + GRAPH_W, GRAPH_Y + GRAPH_H, AXIS_COLOR);
  tft.drawLine(GRAPH_X, GRAPH_Y, GRAPH_X, GRAPH_Y + GRAPH_H, AXIS_COLOR);

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);

  int yDivs = 5;
  for (int i = 0; i <= yDivs; i++) {
    int y = GRAPH_Y + i * (GRAPH_H / yDivs);
    tft.drawLine(GRAPH_X, y, GRAPH_X + GRAPH_W, y, GRID_COLOR);

    float labelVal = maxY - (maxY / yDivs) * i;
    char label[8];
    dtostrf(labelVal, 4, 0, label);
    tft.setCursor(GRAPH_X - 35, y - 5);
    tft.print(label);
  }

  int xDivs = 10;
  for (int i = 0; i <= xDivs; i++) {
    int x = GRAPH_X + i * (GRAPH_W / xDivs);
    tft.drawLine(x, GRAPH_Y, x, GRAPH_Y + GRAPH_H, GRID_COLOR);

    char label[4];
    sprintf(label, "%d", i);
    tft.setCursor(x - 5, GRAPH_Y + GRAPH_H + 5);
    tft.print(label);
  }

  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.setCursor(GRAPH_X + GRAPH_W / 2 - 30, GRAPH_Y + GRAPH_H + 20);
  tft.print("Time (s)");

  uint8_t prevRot = tft.getRotation();
  tft.setRotation((prevRot + 3) % 4);
  tft.setCursor(135, 5);
  tft.print("Stress");
  tft.setRotation(prevRot);
}

void drawGraph(GraphSession& sess) {
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) ID = 0x9486;
  tft.begin(ID);
  tft.setRotation(3);
  if (sess.count < 2) return;

  float minVal = sess.dataPoints[0].value;
  float maxVal = sess.dataPoints[0].value;
  for (int i = 1; i < sess.count; i++) {
    if (sess.dataPoints[i].value < minVal) minVal = sess.dataPoints[i].value;
    if (sess.dataPoints[i].value > maxVal) maxVal = sess.dataPoints[i].value;
  }

  maxVal = ceil(maxVal / 10.0) * 10.0;
  globalMaxY = maxVal;

  uint32_t minTime = sess.dataPoints[0].timestamp;
  uint32_t maxTime = sess.dataPoints[sess.count - 1].timestamp;

  drawAxes(globalMaxY);

  for (int i = 1; i < sess.count; i++) {
    int x0 = GRAPH_X + map(sess.dataPoints[i - 1].timestamp, minTime, maxTime, 0, GRAPH_W);
    int y0 = GRAPH_Y + GRAPH_H - map(sess.dataPoints[i - 1].value, 0, globalMaxY, 0, GRAPH_H);
    int x1 = GRAPH_X + map(sess.dataPoints[i].timestamp, minTime, maxTime, 0, GRAPH_W);
    int y1 = GRAPH_Y + GRAPH_H - map(sess.dataPoints[i].value, 0, globalMaxY, 0, GRAPH_H);

    tft.drawLine(x0, y0, x1, y1, DATA_COLOR);
  }
}