#pragma once

#include "vec2f.h"

#include <vector>
#include <fstream>

namespace mirras
{
struct DrawTarget;

struct Object
{
    virtual void draw(const DrawTarget& drawTarget) const = 0;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) = 0;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const = 0;

    virtual ~Object() = default;
};

struct Point : public Object
{
    Point() = default;
    Point(float _x, float _y) : x(_x), y(_y) {}

    virtual void draw(const DrawTarget& drawTarget) const override;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) override;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const override;

    void transform(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax);

    // Implicitly converts Point to Vec2f
    operator Vec2f() const{ return Vec2f{x, y}; }

    float x{}, y{};   // World Coordinates
    float vX{}, vY{}; // Viewport Coordinates
};

struct LineSegment : public Object
{
    virtual void draw(const DrawTarget& drawTarget) const override;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) override;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const override;

    Point p0{}, p1{};
};

struct Polygon : public Object
{
    Polygon() = default;

    virtual void draw(const DrawTarget& drawTarget) const override;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) override;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const override;

    std::vector<Point> vertices;
};

} // namespace mirras