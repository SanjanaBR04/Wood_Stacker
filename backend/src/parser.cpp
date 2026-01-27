#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include "parser.h"
#include "optimizer.h"

static std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
        tokens.push_back(item);

    return tokens;
}

static std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

std::vector<Panel> parseCSVFiles(const std::vector<std::string>& files,
                                 const std::string& displayName)
{
    std::vector<Panel> panels;

    for (const auto& filename : files)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Cannot open file: " << filename << "\n";
            continue;
        }

        std::string line;

        // Skip header
        std::getline(file, line);

        while (std::getline(file, line))
        {
            if (line.empty()) continue;

            auto cols = split(line, ',');

            if (cols.size() < 4) continue;

            Panel p;
            p.label = trim(cols[0]);
            p.w = std::stod(trim(cols[1]));
            p.h = std::stod(trim(cols[2]));
            p.thickness = std::stod(trim(cols[3]));
            p.x = 0;
            p.y = 0;
            p.dims = trim(cols[1]) + "x" + trim(cols[2]);

            panels.push_back(p);
        }
    }

    return panels;
}
