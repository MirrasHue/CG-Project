#pragma once

// Classes to represent the world and ways to visualize it 

namespace mirras
{
class Window
{
public:
    Vec2f getCenter()
    {
        return (wmin + wmax + Vec2f{wmin.x, wmax.y} + Vec2f{wmax.x, wmin.y}) / 4.f;
    }

    void applyTransform(const glm::mat4& transform)
    {
        Point pwmin = {wmin.x, wmin.y};
        Point pwmax = {wmax.x, wmax.y};

        pwmin.applyTransform(transform);
        pwmax.applyTransform(transform);

        wmin = pwmin;
        wmax = pwmax;
    }

    Vec2f wmin{}, wmax{};
    Vec2f iniWmin{}, iniWmax{};
    float angleRotatedSoFar{0.f};
};

class Viewport
{
public:
    float width{}, height{};
    float borderW{}, borderH{};
};

class World
{
public:
    std::vector<std::unique_ptr<Object>> objects;
};

inline World g_World;
inline Window g_Window;
inline Viewport g_Viewport;

// Used to control the world objects and representation window
inline float translationStep{1.f};
inline float angleStep{10.f};
inline float scaleFactorStep{0.1f};

} // namespace mirras