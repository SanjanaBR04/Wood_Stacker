#include "optimizer.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

// Returns true if rect A is fully contained inside rect B
static bool isContained(const Rect& a, const Rect& b) {
    return a.x >= b.x && a.y >= b.y &&
           a.x + a.w <= b.x + b.w &&
           a.y + a.h <= b.y + b.h;
}

// Remove redundant free rects to keep search space clean
static void pruneFreeRects(std::vector<Rect>& freeRects) {
    for (int i = 0; i < (int)freeRects.size(); ++i) {
        for (int j = 0; j < (int)freeRects.size(); ++j) {
            if (i == j) continue;
            if (isContained(freeRects[i], freeRects[j])) {
                freeRects.erase(freeRects.begin() + i);
                --i;
                break;
            }
        }
    }
}

// MAXRECTS: Clip all free rects against a new placement and split
static void updateFreeRects(std::vector<Rect>& freeRects, double px, double py, double pw, double ph) {
    std::vector<Rect> result;
    result.reserve(freeRects.size() * 2);

    for (const Rect& r : freeRects) {
        if (px >= r.x + r.w || px + pw <= r.x || py >= r.y + r.h || py + ph <= r.y) {
            result.push_back(r);
            continue;
        }
        if (px > r.x) result.push_back({r.x, r.y, px - r.x, r.h});
        if (px + pw < r.x + r.w) result.push_back({px + pw, r.y, r.x + r.w - (px + pw), r.h});
        if (py > r.y) result.push_back({r.x, r.y, r.w, py - r.y});
        if (py + ph < r.y + r.h) result.push_back({r.x, py + ph, r.w, r.y + r.h - (py + ph)});
    }
    freeRects = std::move(result);
    pruneFreeRects(freeRects);
}

static double calcEfficiency(const Sheet& s, double sheetW, double sheetH) {
    double used = 0;
    for (const auto& p : s.parts) used += p.w * p.h;
    return (used / (sheetW * sheetH)) * 100.0;
}

// Standard optimization loop with Sheet Index Penalty for back-filling
std::vector<Sheet> runOptimization(std::vector<Panel>& panels, double sheetW, double sheetH) {
    std::vector<Sheet> sheets;
    std::vector<std::string> unfittableLabels;
    const double epsilon = 0.5;

    std::vector<Panel> remaining = panels;
    std::sort(remaining.begin(), remaining.end(), [](const Panel& a, const Panel& b) {
        return (a.w * a.h) > (b.w * b.h);
    });

    for (auto& p : remaining) {
        int bestSheet = -1, bestRect = -1, bestType = 1;
        double bestScore = std::numeric_limits<double>::max();

        for (int si = 0; si < (int)sheets.size(); ++si) {
            for (int ri = 0; ri < (int)sheets[si].freeRects.size(); ++ri) {
                const Rect& r = sheets[si].freeRects[ri];
                for (int type : {1, 2}) {
                    double pw = (type == 1) ? p.w : p.h;
                    double ph = (type == 1) ? p.h : p.w;

                    if (pw <= r.w + epsilon && ph <= r.h + epsilon) {
                        // Penalty (si * 1000) forces pieces to earlier sheets
                        double score = (r.w * r.h - pw * ph) + (si * 1000.0);
                        if (score < bestScore) {
                            bestScore = score; bestSheet = si; bestRect = ri; bestType = type;
                        }
                    }
                }
            }
        }

        if (bestSheet != -1) {
            if (bestType == 2) { std::swap(p.w, p.h); p.rotated = true; p.label += " (R)"; }
            Rect r = sheets[bestSheet].freeRects[bestRect];
            p.x = r.x; p.y = r.y;
            updateFreeRects(sheets[bestSheet].freeRects, p.x, p.y, p.w, p.h);
            sheets[bestSheet].parts.push_back(p);
        } else {
            bool fitsN = (p.w <= sheetW + epsilon && p.h <= sheetH + epsilon);
            bool fitsR = (p.h <= sheetW + epsilon && p.w <= sheetH + epsilon);
            if (fitsN || fitsR) {
                Sheet ns; ns.freeRects.push_back({0, 0, sheetW, sheetH});
                if (!fitsN || (fitsR && p.h > p.w)) { std::swap(p.w, p.h); p.rotated = true; p.label += " (R)"; }
                p.x = 0; p.y = 0;
                updateFreeRects(ns.freeRects, 0, 0, p.w, p.h);
                ns.parts.push_back(p);
                sheets.push_back(std::move(ns));
            } else {
                unfittableLabels.push_back(p.label);
            }
        }
    }

    if (!unfittableLabels.empty()) {
        std::cout << "\n--- UNFITTABLE PANELS ---\n";
        for (const auto& l : unfittableLabels) std::cout << "  " << l << "\n";
    }

    for (auto& s : sheets) s.efficiency = calcEfficiency(s, sheetW, sheetH);
    return sheets;
}