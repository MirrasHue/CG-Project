#pragma once

#include <imgui.h>
#include <pugixml.hpp>
#include <glm/ext/matrix_transform.hpp>

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
            // Initial pos
            window.iniWmin = points[0];
            window.iniWmax = points[1];
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

inline bool saveXMLFile(const char* fileName)
{
    pxml::xml_document doc;

    auto root = doc.append_child("dados");

    // Insert viewport
    auto viewport = root.append_child("viewport");

    auto vpmin = viewport.append_child("vpmin");
    vpmin.append_attribute("x") = g_Viewport.borderW;
    vpmin.append_attribute("y") = g_Viewport.borderH;
    
    auto vpmax = viewport.append_child("vpmax");
    vpmax.append_attribute("x") = g_Viewport.width;
    vpmax.append_attribute("y") = g_Viewport.height;

    // Insert window
    auto window = root.append_child("window");

    auto wmin = window.append_child("wmin");
    wmin.append_attribute("x") = g_Window.wmin.x;
    wmin.append_attribute("y") = g_Window.wmin.y;
    
    auto wmax = window.append_child("wmax");
    wmax.append_attribute("x") = g_Window.wmax.x;
    wmax.append_attribute("y") = g_Window.wmax.y;

    // Insert objects
    for(const auto& obj : g_World.objects)
    {
        if(auto point = dynamic_cast<Point*>(obj.get()))
        {
            auto p = root.append_child("ponto");

            p.append_attribute("x") = point->x;
            p.append_attribute("y") = point->y;
        }
        else if(auto line = dynamic_cast<LineSegment*>(obj.get()))
        {
            auto ln = root.append_child("reta");

            auto p0 = ln.append_child("ponto");
            p0.append_attribute("x") = line->p0.x;
            p0.append_attribute("y") = line->p0.y;

            auto p1 = ln.append_child("ponto");
            p1.append_attribute("x") = line->p1.x;
            p1.append_attribute("y") = line->p1.y;
        }
        else if(auto polygon = dynamic_cast<Polygon*>(obj.get()))
        {
            auto poly = root.append_child("poligono");

            for(const auto& point : polygon->vertices)
            {
                auto p = poly.append_child("ponto");
                p.append_attribute("x") = point.x;
                p.append_attribute("y") = point.y;
            }
        }
    }

    return doc.save_file(fileName);
}


///////////////  Graphics Utilities  /////////////////

enum class ButtonType
{
    Button,
    Arrow,
    Small
};

inline void offsetCursorPosX(float availWidth, float itemWidth, float alignment)
{
    float offset = (availWidth - itemWidth) * alignment;

    if(offset > 0.f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
}

inline bool ImGuiAlignedButton(ButtonType buttonType, const char* label, float alignment, ImGuiDir dir = ImGuiDir_None)
{
    ImGuiStyle& style = ImGui::GetStyle();

    float availX = ImGui::GetContentRegionAvail().x;
    float sizeX = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;

    switch(buttonType)
    {
    case ButtonType::Button:
    {
        offsetCursorPosX(availX, sizeX, alignment);

        return ImGui::Button(label);
    }
    case ButtonType::Arrow:
    {
        float arrowWidth = style.FramePadding.x * 5.f; // Special case for arrow button, as I don't know its width

        offsetCursorPosX(availX, arrowWidth, alignment);

        return ImGui::ArrowButton(label, dir);
    }
    case ButtonType::Small:
    {
        offsetCursorPosX(availX, sizeX, alignment);

        return ImGui::SmallButton(label);
    }
    default:
        assert(false && "Invalid button type!");
    }

    return false;
}

inline glm::mat4 rotateAroundCenter(Vec2f center, float angle)
{
    auto t1 = glm::translate(glm::mat4(1.f), glm::vec3(-center.x, -center.y, 0.f));
    auto rot = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f));
    auto t2 = glm::translate(glm::mat4(1.f), glm::vec3(center.x, center.y, 0.f));

    return t2 * rot * t1;
}

inline glm::mat4 scaleAroundCenter(Vec2f center, float scaleFactor)
{
    auto t1 = glm::translate(glm::mat4(1.f), glm::vec3(-center.x, -center.y, 0.f));
    auto s  = glm::scale(glm::mat4(1.f), glm::vec3(scaleFactor, scaleFactor, 0.f));
    auto t2 = glm::translate(glm::mat4(1.f), glm::vec3(center.x, center.y, 0.f));

    return t2 * s * t1;
}

inline Vec2f g_WinCenter;

inline void rotateWindow(float angle)
{
    if(g_Window.angleRotatedSoFar == 0.f)
        g_WinCenter = g_Window.getCenter();

    Vec2f winCenter = g_Window.getCenter();

    // Calculate PPC
    auto t = glm::translate(glm::mat4(1.f), glm::vec3(-winCenter.x, -winCenter.y, 0.f));
    auto rot = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0.f, 0.f, 1.f));

    auto ppc = rot * t;

    g_Window.applyTransform(t);

    // Apply PPC to all objects
    for(auto& obj : g_World.objects)
        obj->applyTransform(ppc);

    g_Window.angleRotatedSoFar += angle;
}

inline void scaleWindow(float scaleFactor)
{
    Vec2f winCenter = g_Window.getCenter();

    auto scale = scaleAroundCenter(winCenter, scaleFactor);

    g_Window.applyTransform(scale);
}

inline void resetWindow()
{   
    g_Window.wmin = g_Window.iniWmin;
    g_Window.wmax = g_Window.iniWmax;

    if(g_Window.angleRotatedSoFar == 0.f)
        return;

    auto t = glm::translate(glm::mat4(1.f), glm::vec3(g_WinCenter.x, g_WinCenter.y, 0.f));
    auto rot = glm::rotate(glm::mat4(1.f), glm::radians(g_Window.angleRotatedSoFar), glm::vec3(0.f, 0.f, 1.f));

    auto invPPC = t * rot;

    for(auto& obj : g_World.objects)
        obj->applyTransform(invPPC);

    g_Window.angleRotatedSoFar = 0.f;
}

} // namespace mirras