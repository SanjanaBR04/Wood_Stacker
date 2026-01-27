#define CPPHTTPLIB_MULTIPART_FORM_DATA

#include "httplib.h"
#include "parser.h"
#include "optimizer.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

std::string jsonEscape(const std::string& s)
{
    std::string out;
    for (unsigned char c : s)
    {
        switch (c)
        {
        case '\"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c >= 32) out += c;
        }
    }
    return out;
}

std::string buildJSON(const std::vector<Sheet>& sheets)
{
    std::ostringstream os;
    os << "{ \"sheets\": [";

    for (size_t i = 0; i < sheets.size(); ++i)
    {
        const auto& s = sheets[i];
        os << "{ \"efficiency\": " << s.efficiency << ", \"parts\": [";

        for (size_t j = 0; j < s.parts.size(); ++j)
        {
            const auto& p = s.parts[j];
            os << "{"
               << "\"x\":" << p.x << ","
               << "\"y\":" << p.y << ","
               << "\"w\":" << p.w << ","
               << "\"h\":" << p.h << ","
               << "\"label\":\"" << jsonEscape(p.label) << "\","
               << "\"dims\":\"" << jsonEscape(p.dims) << "\""
               << "}";

            if (j + 1 < s.parts.size()) os << ",";
        }

        os << "]}";
        if (i + 1 < sheets.size()) os << ",";
    }

    os << "] }";
    return os.str();
}

int main()
{
    httplib::Server server;

    server.Options("/calculate", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "*");
        res.status = 204;
    });

    server.Post("/calculate", [](const httplib::Request& req, httplib::Response& res)
    {
        res.set_header("Access-Control-Allow-Origin", "*");

        if (!req.is_multipart_form_data()) {
            res.status = 400;
            res.set_content("Not multipart", "text/plain");
            return;
        }

        if (req.form.fields.count("sheetWidth") == 0 ||
            req.form.fields.count("sheetHeight") == 0)
        {
            res.status = 400;
            res.set_content("Missing sheet dimensions", "text/plain");
            return;
        }

        if (req.form.files.empty()) {
            res.status = 400;
            res.set_content("No CSV files uploaded", "text/plain");
            return;
        }

        double sheetW = std::stod(req.form.fields.find("sheetWidth")->second.content);
        double sheetH = std::stod(req.form.fields.find("sheetHeight")->second.content);

        std::vector<Panel> allPanels;

        for (const auto& item : req.form.files)
        {
            const auto& file = item.second;

            std::string tempPath =
                (std::filesystem::temp_directory_path() / file.filename).string();

            std::ofstream out(tempPath, std::ios::binary);
            out << file.content;
            out.close();

            std::vector<std::string> files = { tempPath };
            auto panels = parseCSVFiles(files, file.filename);

            allPanels.insert(allPanels.end(), panels.begin(), panels.end());
        }

        // -------- GROUP BY THICKNESS --------
        std::map<double, std::vector<Panel>> grouped;

        for (const auto& p : allPanels)
            grouped[p.thickness].push_back(p);

        std::ostringstream finalJson;
        finalJson << "{ \"groups\": [";

        bool firstGroup = true;

        for (auto& g : grouped)
        {
            if (!firstGroup) finalJson << ",";
            firstGroup = false;

            double thickness = g.first;
            auto sheets = runOptimization(g.second, sheetW, sheetH);

            finalJson << "{";
            finalJson << "\"thickness\":" << thickness << ",";
            finalJson << "\"result\":" << buildJSON(sheets);
            finalJson << "}";
        }

        finalJson << "] }";

        res.set_content(finalJson.str(), "application/json");
    });

    std::cout << "Optimizer Server running at http://localhost:8000\n";
    server.listen("0.0.0.0", 8000);
}
