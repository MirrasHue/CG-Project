#pragma once

#include "vec2f.h"

#include <vector>
#include <fstream>

#include <glm/mat4x4.hpp>

namespace mirras
{
struct DrawTarget;
class Window;

struct Object
{
    virtual void draw(const DrawTarget& drawTarget) const = 0;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) = 0;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const = 0;
    virtual void applyTransform(const glm::mat4& transform) = 0;
    virtual Vec2f getCenter() const = 0;
    virtual const char* getTypeName() const = 0;
    virtual bool isInside(const Window& win) const = 0;

    bool isSelected{};

    virtual ~Object() = default;
};

struct Point : public Object
{
    Point() = default;
    Point(float _x, float _y) : x(_x), y(_y) {}

    virtual void draw(const DrawTarget& drawTarget) const override;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) override;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const override;
    virtual void applyTransform(const glm::mat4& transform) override;
    virtual Vec2f getCenter() const override;
    virtual bool isInside(const Window& win) const override;

    virtual const char* getTypeName() const
    {
        return "Point";
    }

    void toViewport(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax);

    // Implicitly converts Point to Vec2f
    operator Vec2f() const{ return Vec2f{x, y}; }

    float x{}, y{};   // World Coordinates
    float vX{}, vY{}; // Viewport Coordinates
    static inline uint32_t color{};
};

struct LineSegment : public Object
{
    virtual void draw(const DrawTarget& drawTarget) const override;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) override;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const override;
    virtual void applyTransform(const glm::mat4& transform) override;
    virtual Vec2f getCenter() const override;
    virtual bool isInside(const Window& win) const override;

    virtual const char* getTypeName() const
    {
        return "Line";
    }

    Point p0{}, p1{};
    static inline uint32_t color{};
};

struct Polygon : public Object
{
    Polygon() = default;
    Polygon(std::vector<Point> _vertices) : vertices(std::move(_vertices)) {}

    virtual void draw(const DrawTarget& drawTarget) const override;
    virtual void toViewportCoord(Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax) override;
    virtual void writeViewportCoordToFile(std::ofstream& outputFile) const override;
    virtual void applyTransform(const glm::mat4& transform) override;
    virtual Vec2f getCenter() const override;
    virtual bool isInside(const Window& win) const override;

    virtual const char* getTypeName() const
    {
        return "Polygon";
    }

    std::vector<Point> vertices;
    static inline uint32_t color{};
};

} // namespace mirras