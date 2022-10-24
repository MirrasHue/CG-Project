#pragma once

namespace mirras
{
// Simple data structure to represent a generic position, instead of using Point
struct Vec2f
{
    float x{}, y{};
};

inline Vec2f operator+ (Vec2f lhs, Vec2f rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline Vec2f operator- (Vec2f lhs, Vec2f rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline Vec2f operator* (Vec2f lhs, float n)
{
    return {lhs.x * n, lhs.y * n};
}

inline Vec2f operator/ (Vec2f lhs, float n)
{
    return {lhs.x / n, lhs.y / n};
}

inline bool operator== (Vec2f lhs, Vec2f rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y);
}

} // namespace mirras