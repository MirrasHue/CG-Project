#include "objects.h"

#include "graphics.h"
//#include <iostream>

namespace mirras
{
///////////////  Point  /////////////////
void Point::draw(const DrawTarget& target) const
{
    // Workaround to draw a point
    Vec2f vP = {vX, vY};
    target.draw_list->AddCircle(vP + target.cursorPos, 2.f, target.color, 0, target.thickness);
}

void Point::toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    transform(wmin, wmax, vmin, vmax);
}

void Point::transform(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    vX = (x - wmin.x) / (wmax.x - wmin.x) * (vmax.x) + vmin.x;
    vY = (1 - (y - wmin.y) / (wmax.y - wmin.y)) * (vmax.y) + vmin.y;
}

void Point::writeViewportCoordToFile(std::ofstream& outputFile) const
{
    outputFile << "Point:    " << vX << "   " << vY << '\n';
}

///////////////  Line Segment  /////////////////
void LineSegment::draw(const DrawTarget& target) const
{
    //printf("(%.2f %.2f), (%.2f %.2f)\n", p0.vX, p0.vY, p1.vX, p1.vY);
    Vec2f vP0 = {p0.vX, p0.vY};
    Vec2f vP1 = {p1.vX, p1.vY};
    target.draw_list->AddLine(vP0 + target.cursorPos, vP1 + target.cursorPos, target.color, target.thickness);
}

void LineSegment::toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    p0.transform(wmin, wmax, vmin, vmax);
    p1.transform(wmin, wmax, vmin, vmax);
}

void LineSegment::writeViewportCoordToFile(std::ofstream& outputFile) const
{
    outputFile << "Line:\n          " << p0.vX << "   " << p0.vY << '\n';
    outputFile <<        "          " << p1.vX << "   " << p1.vY << '\n';
}

///////////////  Polygon  /////////////////
void Polygon::draw(const DrawTarget& target) const
{
    std::vector<ImVec2> pointsWithOffset;
    pointsWithOffset.reserve(vertices.size());

    for(auto p : vertices)
        pointsWithOffset.push_back(Vec2f{p.vX, p.vY} + target.cursorPos);

    target.draw_list->AddPolyline(pointsWithOffset.data(), vertices.size(), target.color, ImDrawFlags_Closed, target.thickness);
}

void Polygon::toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    for(auto& p : vertices)
        p.transform(wmin, wmax, vmin, vmax);
}

void Polygon::writeViewportCoordToFile(std::ofstream& outputFile) const
{
    outputFile << "Polygon:\n";
    for(const auto& p : vertices)
        outputFile << "          " << p.vX << "   " << p.vY << '\n';
}

} // namespace mirras