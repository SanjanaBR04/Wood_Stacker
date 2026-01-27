#pragma once
#include <vector>
#include <string>
#include "optimizer.h"

std::vector<Panel> parseCSVFiles(
    const std::vector<std::string>& files,
    const std::string& originalName
);
