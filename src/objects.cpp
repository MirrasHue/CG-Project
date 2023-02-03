#include "objects.h"

#include "graphics.h"
#include "clippingAlgorithms.h"

//#include <iostream>

namespace mirras
{
///////////////  Point  /////////////////
void Point::draw(const DrawTarget& target) const
{
    Vec2f vP = {vX, vY};
    uint32_t tempColor = isSelected ? IM_COL32_WHITE : color;

    // Workaround to draw a point
    target.draw_list->AddCircle(vP + target.currentDrawPos, 2.f, tempColor, 0, target.thickness);
}

void Point::toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    toViewport(wmin, wmax, vmin, vmax);
}

void Point::toViewport(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    vX = (x - wmin.x) / (wmax.x - wmin.x) * (vmax.x) + vmin.x;
    vY = (1 - (y - wmin.y) / (wmax.y - wmin.y)) * (vmax.y) + vmin.y;
}

void Point::writeViewportCoordToFile(std::ofstream& outputFile) const
{
    outputFile << "Point:    " << vX << "   " << vY << '\n';
}

void Point::applyTransform(const glm::mat4& transform)
{
    auto result = transform * glm::vec4(x, y, 0.f, 1.f);
    x = result.x;
    y = result.y;
}

Vec2f Point::getCenter() const
{
    return {x, y};
}

bool Point::isInside(const Window& win) const
{
    if(x <= win.wmax.x && x >= win.wmin.x && y <= win.wmax.y && y >= win.wmin.y)
        return true;

    return false;
}

///////////////  Line Segment  /////////////////
void LineSegment::draw(const DrawTarget& target) const
{
    uint32_t tempColor = isSelected ? IM_COL32_WHITE : color;

    Vec2f vmin = {g_Viewport.borderW, g_Viewport.borderH};
    Vec2f vmax = {g_Viewport.width, g_Viewport.height};

    std::optional<LineSeg> line;

    if(target.enableCohenSutherland)
        line = cohenSutherland(g_Window, LineSeg{p0, p1});
    else
    if(target.enableLiangBarsky)
        line = liangBarsky(g_Window, LineSeg{p0, p1});
    else
    {
        Vec2f vP0 = {p0.vX, p0.vY};
        Vec2f vP1 = {p1.vX, p1.vY};

        target.draw_list->AddLine(vP0 + target.currentDrawPos, vP1 + target.currentDrawPos, tempColor, target.thickness);

        return;
    }

    if(line)
    {
        Point p0{line->p0.x, line->p0.y};
        Point p1{line->p1.x, line->p1.y};

        p0.toViewport(g_Window.wmin, g_Window.wmax, vmin, vmax);
        p1.toViewport(g_Window.wmin, g_Window.wmax, vmin, vmax);

        Vec2f vP0 = {p0.vX, p0.vY};
        Vec2f vP1 = {p1.vX, p1.vY};

        target.draw_list->AddLine(vP0 + target.currentDrawPos, vP1 + target.currentDrawPos, tempColor, target.thickness);        
    }
}

void LineSegment::toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    p0.toViewport(wmin, wmax, vmin, vmax);
    p1.toViewport(wmin, wmax, vmin, vmax);
}

void LineSegment::writeViewportCoordToFile(std::ofstream& outputFile) const
{
    outputFile << "Line:\n          " << p0.vX << "   " << p0.vY << '\n';
    outputFile <<        "          " << p1.vX << "   " << p1.vY << '\n';
}

void LineSegment::applyTransform(const glm::mat4& transform)
{
    p0.applyTransform(transform);
    p1.applyTransform(transform);
}

Vec2f LineSegment::getCenter() const
{
    return (p0 + p1) / 2.f;
}

bool LineSegment::isInside(const Window& win) const
{
    if(p0.isInside(win) && p1.isInside(win))
        return true;

    return false;
}

///////////////  Polygon  /////////////////
void Polygon::draw(const DrawTarget& target) const
{
    std::vector<ImVec2> pointsWithOffset;
    pointsWithOffset.reserve(vertices.size());

    uint32_t tempColor = isSelected ? IM_COL32_WHITE : color;
    bool isInsideWindow{};

    if(isInside(g_Window))
        isInsideWindow = true;

    if(!target.enableWeilerAtherton || isInsideWindow)
    {
        for(const auto& p : vertices)
            pointsWithOffset.emplace_back(Vec2f{p.vX, p.vY} + target.currentDrawPos);

        target.draw_list->AddPolyline(pointsWithOffset.data(), vertices.size(), tempColor, ImDrawFlags_Closed, target.thickness);
    }
    else
    {
        auto subPolygons = weilerAtherton(*this, g_Window);

        Vec2f vmin = {g_Viewport.borderW, g_Viewport.borderH};
        Vec2f vmax = {g_Viewport.width, g_Viewport.height};

        for(const auto& subPoly : subPolygons)
        {
            pointsWithOffset.clear();

            for(const auto& vert : subPoly)
            {
                Point p{vert.pos.x, vert.pos.y};
                p.toViewport(g_Window.wmin, g_Window.wmax, vmin, vmax);

                pointsWithOffset.emplace_back(Vec2f{p.vX, p.vY} + target.currentDrawPos);
            }
            target.draw_list->AddPolyline(pointsWithOffset.data(), pointsWithOffset.size(), tempColor, ImDrawFlags_Closed, target.thickness);
        }
    }
}

void Polygon::toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    for(auto& p : vertices)
        p.toViewport(wmin, wmax, vmin, vmax);
}

void Polygon::writeViewportCoordToFile(std::ofstream& outputFile) const
{
    outputFile << "Polygon:\n";
    for(const auto& p : vertices)
        outputFile << "          " << p.vX << "   " << p.vY << '\n';
}

void Polygon::applyTransform(const glm::mat4& transform)
{
    for(auto& p : vertices)
        p.applyTransform(transform);
}

Vec2f Polygon::getCenter() const
{
    Vec2f sum{};
    for(const auto& p : vertices)
        sum = sum + p;

    return sum / (float) vertices.size();
}

bool Polygon::isInside(const Window& win) const
{
    for(const auto& p : vertices)
    {
        if(p.isInside(win))
            continue;
        else
            return false;
    }

    return true;
}

} // namespace mirras