#pragma once

#include "graphics.h"

namespace mirras
{
class App
{
public:
    App(int windowWidth = 800, int windowHeight = 600, const char* title = "CG-Project")
    {
        initGLFW();
        glfw::WindowHints{.contextVersionMajor = 3,
                          .contextVersionMinor = 3,
                          .openglProfile = glfw::OpenGlProfile::Core
                         }.apply();

        glfw::Window tempWindow{windowWidth, windowHeight, title};

        window = std::move(tempWindow);

        glfw::makeContextCurrent(window);
        glfw::swapInterval(1); // Enable v-sync

        initGlad();
        initImGui(window);
    }

    ~App()
    {
        shutdownImGui();
        glfw::terminate();
    }

    void run()
    {
        while(!window.shouldClose())
        {
            glfw::pollEvents();

            auto[width, height] = window.getFramebufferSize();
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);
            
            renderImGui();
            window.swapBuffers();
        }
    }

private:
    glfw::Window window;
};
    
} // namespace mirras


