#pragma once
#include <vector>
#include <string>

struct Rect {
    double x = 0, y = 0, w = 0, h = 0;
};

struct Panel {
    std::string label;
    std::string dims;   // original "WxH" string for display
    double x = 0, y = 0;
    double w = 0, h = 0;
    double thickness = 0;
    bool   rotated = false;
};

struct Sheet {
    std::vector<Panel> parts;
    std::vector<Rect>  freeRects;
    double efficiency = 0.0;
};

std::vector<Sheet> runOptimization(std::vector<Panel>& panels,
                                   double sheetW, double sheetH);