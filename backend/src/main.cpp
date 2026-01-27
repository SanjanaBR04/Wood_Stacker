#include <iostream>
#include <vector>
#include <string>
#include "parser.h"
#include "optimizer.h"

int main(int argc, char* argv[])
{
    std::cout << "Program started\n";

    if (argc < 4) {
        std::cout << "Usage: optimizer sheetWidth sheetHeight file1.csv ...\n";
        return 1;
    }

    double sheetW = std::stod(argv[1]);
    double sheetH = std::stod(argv[2]);

    std::vector<std::string> files;
    for (int i = 3; i < argc; ++i)
        files.push_back(argv[i]);

    auto panels = parseCSVFiles(files, "CLI");
    auto sheets = runOptimization(panels, sheetW, sheetH);

    std::cout << "Sheets used: " << sheets.size() << "\n";

    return 0;
}
