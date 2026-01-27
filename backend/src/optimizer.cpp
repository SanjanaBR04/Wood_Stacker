#include "optimizer.h"
#include <algorithm>
#include <limits>

static bool fits(const Rect& r, const Panel& p) {
    return p.w <= r.w && p.h <= r.h;
}

static void splitRect(std::vector<Rect>& freeRects, const Rect& used, const Panel& p) {

    Rect right{
        used.x + p.w,
        used.y,
        used.w - p.w,
        p.h
    };

    Rect bottom{
        used.x,
        used.y + p.h,
        used.w,
        used.h - p.h
    };

    if (right.w > 0 && right.h > 0)
        freeRects.push_back(right);

    if (bottom.w > 0 && bottom.h > 0)
        freeRects.push_back(bottom);
}


static bool placeInSheet(Sheet& sheet, Panel& p) {

    int bestIndex = -1;

    double bestArea = std::numeric_limits<double>::max();

    for (size_t i = 0; i < sheet.freeRects.size(); ++i) {

        const auto& r = sheet.freeRects[i];

        if (fits(r, p)) {

            double area = r.w * r.h;

            if (area < bestArea) {
                bestArea = area;
                bestIndex = (int)i;
            }
        }
    }

    if (bestIndex == -1)
        return false;

    Rect chosen = sheet.freeRects[bestIndex];

    sheet.freeRects.erase(sheet.freeRects.begin() + bestIndex);

    p.x = chosen.x;
    p.y = chosen.y;

    sheet.parts.push_back(p);
    splitRect(sheet.freeRects, chosen, p);

    return true;
}


static double calcEfficiency(const Sheet& s, double w, double h) {
    double used = 0;
    for (const auto& p : s.parts)
        used += p.w * p.h;

    return (used / (w * h)) * 100.0;
}

std::vector<Sheet> runOptimization(std::vector<Panel>& panels,
                                double sheetW,
                                double sheetH)
{

    std::sort(panels.begin(), panels.end(),
        [](const Panel& a, const Panel& b) {
            return (a.w * a.h) > (b.w * b.h);
        });

    std::vector<Sheet> sheets;

    for (auto& p : panels)
    {
        bool placed = false;

        for (auto& s : sheets) {
            if (placeInSheet(s, p)) {
                placed = true;
                break;
            }
        }

        if (!placed) {
            Sheet newSheet;

            newSheet.freeRects.push_back({0, 0, sheetW, sheetH});

            if (placeInSheet(newSheet, p)) {
                sheets.push_back(newSheet);
            }
        }
    }

    for (auto& s : sheets)
        s.efficiency = calcEfficiency(s, sheetW, sheetH);

    return sheets;
}
