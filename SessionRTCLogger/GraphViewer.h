#ifndef GRAPH_VIEWER_H
#define GRAPH_VIEWER_H

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>


// --------------------------
// Data structures
// --------------------------
struct GraphPoint {
  uint32_t timestamp; // milliseconds
  float value;
};

class GraphSession {
public:
  static const int MAX_POINTS = 50;
  GraphPoint dataPoints[MAX_POINTS];
  int count = 0;

  void addPoint(float val, float seconds);
  void reset();
};

extern GraphSession graphSession;

void drawAxes(float maxY);
void drawGraph(GraphSession& sess);

#endif