#include "objects.h"

#include "graphics.h"
//#include <iostream>

namespace mirras
{
///////////////  Point  /////////////////
void Point::draw(const DrawTarget& target) const
{
    Vec2f vP = {vX, vY};
    uint32_t tempColor = bIsSelected ? IM_COL32_WHITE : color;

    // Workaround to draw a point
    target.draw_list->AddCircle(vP + target.cursorPos, 2.f, tempColor, 0, target.thickness);
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

///////////////  Line Segment  /////////////////
void LineSegment::draw(const DrawTarget& target) const
{
    //printf("(%.2f %.2f), (%.2f %.2f)\n", p0.vX, p0.vY, p1.vX, p1.vY);
    Vec2f vP0 = {p0.vX, p0.vY};
    Vec2f vP1 = {p1.vX, p1.vY};
    uint32_t tempColor = bIsSelected ? IM_COL32_WHITE : color;

    target.draw_list->AddLine(vP0 + target.cursorPos, vP1 + target.cursorPos, tempColor, target.thickness);
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

///////////////  Polygon  /////////////////
void Polygon::draw(const DrawTarget& target) const
{
    std::vector<ImVec2> pointsWithOffset;
    pointsWithOffset.reserve(vertices.size());

    for(const auto& p : vertices)
        pointsWithOffset.emplace_back(Vec2f{p.vX, p.vY} + target.cursorPos);

    uint32_t tempColor = bIsSelected ? IM_COL32_WHITE : color;

    target.draw_list->AddPolyline(pointsWithOffset.data(), vertices.size(), tempColor, ImDrawFlags_Closed, target.thickness);
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

} // namespace mirras