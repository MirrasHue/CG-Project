#pragma once

// Classes to represent the world and ways to visualize it 

namespace mirras
{
class Window
{
public:
    Vec2f wmin{}, wmax{};
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

} // namespace mirras