#include "optimizer.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <vector>
#include <string>

// Helper to check if one rectangle is completely contained within another
static bool isContained(const Rect& a, const Rect& b) {
    return a.x >= b.x && a.y >= b.y &&
           a.x + a.w <= b.x + b.w && a.y + a.h <= b.y + b.h;
}

// MaxRects Split: Breaks free space into overlapping rectangles to maximize gap visibility
static void performSplit(std::vector<Rect>& freeRects, Rect free, const Panel& p) {
    if (p.x >= free.x + free.w || p.x + p.w <= free.x ||
        p.y >= free.y + free.h || p.y + p.h <= free.y)
        return;

    if (p.y > free.y) 
        freeRects.push_back({free.x, free.y, free.w, p.y - free.y});
    if (p.y + p.h < free.y + free.h) 
        freeRects.push_back({free.x, p.y + p.h, free.w, (free.y + free.h) - (p.y + p.h)});
    if (p.x > free.x) 
        freeRects.push_back({free.x, free.y, p.x - free.x, free.h});
    if (p.x + p.w < free.x + free.w) 
        freeRects.push_back({p.x + p.w, free.y, (free.x + free.w) - (p.x + p.w), free.h});
}

// HEURISTIC: Best Short-Side Fit (BSSF) with Height Weighting
// This minimizes waste by finding the snug-fitting hole for each piece
static bool placeInSheet(Sheet& sheet, Panel& p) {
    int bestIndex = -1;
    int fitType = 0; 
    double bestScore = std::numeric_limits<double>::max();

    for (size_t i = 0; i < sheet.freeRects.size(); ++i) {
        const auto& r = sheet.freeRects[i];
        for (int type : {1, 2}) {
            double pw = (type == 1) ? p.w : p.h;
            double ph = (type == 1) ? p.h : p.w;

            if (pw <= r.w && ph <= r.h) {
                double leftoverW = std::abs(r.w - pw);
                double leftoverH = std::abs(r.h - ph);
                
                // Weighting height waste more heavily encourages systematic rows
                double score = (std::min(leftoverW, leftoverH) * 1.0) + (std::max(leftoverW, leftoverH) * 0.1);
                
                if (score < bestScore) {
                    bestScore = score;
                    bestIndex = (int)i;
                    fitType = type;
                }
            }
        }
    }

    if (bestIndex == -1) return false;
    if (fitType == 2) { 
        std::swap(p.w, p.h); 
        p.dims += " (R)"; 
    }

    p.x = sheet.freeRects[bestIndex].x;
    p.y = sheet.freeRects[bestIndex].y;

    std::vector<Rect> nextFreeRects;
    for (const auto& f : sheet.freeRects) performSplit(nextFreeRects, f, p);

    sheet.freeRects.clear();
    for (size_t i = 0; i < nextFreeRects.size(); ++i) {
        bool skip = false;
        for (size_t j = 0; j < nextFreeRects.size(); ++j) {
            if (i != j && isContained(nextFreeRects[i], nextFreeRects[j])) { 
                skip = true; break; 
            }
        }
        if (!skip) {
            bool duplicate = false;
            for(const auto& f : sheet.freeRects) {
                if(f.x == nextFreeRects[i].x && f.y == nextFreeRects[i].y && 
                   f.w == nextFreeRects[i].w && f.h == nextFreeRects[i].h) {
                    duplicate = true; break;
                }
            }
            if(!duplicate) sheet.freeRects.push_back(nextFreeRects[i]);
        }
    }

    sheet.parts.push_back(p);
    return true;
}

// Function to calculate the efficiency of a single sheet
static double calcEfficiency(const Sheet& s, double w, double h) {
    double used = 0;
    for (const auto& p : s.parts) 
        used += p.w * p.h;
    return (used / (w * h)) * 100.0;
}

// Internal simulation to test a specific sort order across sheets
static std::vector<Sheet> runSimulation(std::vector<Panel> panels, double sheetW, double sheetH) {
    std::vector<Sheet> sheets;
    for (auto& p : panels) {
        bool placed = false;
        // Search all existing sheets to fill gaps before creating a new one
        for (auto& s : sheets) {
            if (placeInSheet(s, p)) { 
                placed = true; break; 
            }
        }
        if (!placed) {
            Sheet ns;
            ns.freeRects.push_back({0, 0, sheetW, sheetH});
            if (placeInSheet(ns, p)) 
                sheets.push_back(ns);
        }
    }
    return sheets;
}

// MAIN ENTRY: Tries multiple strategies to find the absolute minimum sheet count
std::vector<Sheet> runOptimization(std::vector<Panel>& panels, double sheetW, double sheetH) {
    
    // Strategy 1: Area (High Density Packing)
    auto sortArea = panels;
    std::sort(sortArea.begin(), sortArea.end(), [](const Panel& a, const Panel& b) {
        return (a.w * a.h) > (b.w * b.h);
    });
    auto resArea = runSimulation(sortArea, sheetW, sheetH);

    // Strategy 2: Max Dimension (Better for systematic "Anchoring")
    auto sortMax = panels;
    std::sort(sortMax.begin(), sortMax.end(), [](const Panel& a, const Panel& b) {
        return std::max(a.w, a.h) > std::max(b.w, b.h);
    });
    auto resMax = runSimulation(sortMax, sheetW, sheetH);

    // Strategy 3: Width (Systematic Columns)
    auto sortWidth = panels;
    std::sort(sortWidth.begin(), sortWidth.end(), [](const Panel& a, const Panel& b) {
        if (std::abs(a.w - b.w) > 0.1) return a.w > b.w;
        return a.h > b.h;
    });
    auto resWidth = runSimulation(sortWidth, sheetW, sheetH);

    // Select the winner with the fewest total sheets
    std::vector<Sheet> best = resArea;
    if (resMax.size() < best.size()) best = resMax;
    if (resWidth.size() < best.size()) best = resWidth;

    // Apply the efficiency calculation to each sheet in the winning result
    for (auto& s : best) {
        s.efficiency = calcEfficiency(s, sheetW, sheetH);
    }

    return best;
}