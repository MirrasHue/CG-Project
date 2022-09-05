#pragma once

#include <imgui.h>
#include <pugixml.hpp>

#include "imGuiLogger.h"
#include "objects.h"
#include "representation.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;
namespace pxml = pugi;

namespace mirras
{
inline ImGuiLogger g_Logger;

inline fs::path pathToConfigFile(const char* fileName)
{
    auto currentPath = fs::current_path();
    auto filePath = currentPath / fileName;
    int max_n_iter = 10;

    for(int i = 0; i < max_n_iter; ++i)
    {
        // We don't look into sub folders while going back towards the root
        if(fs::exists(filePath))
            return filePath;
        else
        {
            currentPath = currentPath.parent_path();
            filePath = currentPath / fileName;
        }
    }
    return {};
}

inline void writeObjectsVpCoordToFile(std::ofstream& outputFile)
{
    for(const auto& obj : g_World.objects)
        obj->writeViewportCoordToFile(outputFile);
}

struct XMLParsedData
{
    World world;
    Window window;
    Viewport viewport;
};

inline Point retrievePoint(pxml::xml_node leaf)
{
    Point p;
    auto point = leaf.attributes();
    p.x = point.begin()->as_float();
    p.y = (++point.begin())->as_float();
    
    return p;
}

inline std::vector<Point> retrievePoints(pxml::xml_node parent)
{
    std::vector<Point> points;
    for (auto child : parent.children())
        points.push_back(retrievePoint(child));
    
    return points;
}

inline std::optional<XMLParsedData> loadDataFromXMLFile(const char* filePath)
{
    pxml::xml_document doc;

    if(!doc.load_file(filePath))
    {
        g_Logger.AddLog("Not able to load the file!\n");

        return {};
    }

    World world;
    Window window;
    Viewport viewport;

    g_Logger.AddLog("Parsing the file...\n");

    for (auto child : doc.child("dados").children())
    {
        std::string_view name{child.name()};
        g_Logger.AddLog("\t%s\n", child.name());

        if(name == "ponto")
        {
            Point p = retrievePoint(child);
            world.objects.emplace_back(std::make_unique<Point>(p));
        }
        else if(name == "reta")
        {
            LineSegment line;
            auto points = retrievePoints(child);
            line.p0 = points[0];
            line.p1 = points[1];
            world.objects.emplace_back(std::make_unique<LineSegment>(line));
        }
        else if(name == "poligono")
        {
            Polygon poly;
            poly.vertices = retrievePoints(child);
            world.objects.emplace_back(std::make_unique<Polygon>(std::move(poly)));
        }
        else if(name == "viewport")
        {
            auto points = retrievePoints(child);
            viewport.borderW = points[0].x;
            viewport.borderH = points[0].y;
            viewport.width = points[1].x;
            viewport.height = points[1].y;
        }
        else if(name == "window")
        {
            auto points = retrievePoints(child);
            window.wmin = points[0];
            window.wmax = points[1];
        }
        else
        {
            g_Logger.AddLog("\nInvalid XML format!\n\n");
            
            return {};
        }
    }

    g_Logger.AddLog("Parse completed!\n");
    
    return XMLParsedData{std::move(world), window, viewport};
}

} // namespace mirras