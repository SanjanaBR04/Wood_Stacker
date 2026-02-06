#pragma once
#include <vector>
#include <string>

struct Panel {
    double w;
    double h;
    double x = 0;
    double y = 0;
    std::string label;
    std::string dims;
    double thickness;
    bool rotated = false;
};

struct Rect {
    double x, y, w, h;
};

struct Sheet {
    std::vector<Panel> parts;
    std::vector<Rect> freeRects;   
    double efficiency = 0;
};

std::vector<Sheet> runOptimization(std::vector<Panel>& panels,
                                   double sheetW,
                                   double sheetH);
