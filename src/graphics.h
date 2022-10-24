#pragma once

#include <glad/glad.h>
#include <glfwpp/glfwpp.h>
#include <imgui.h>
#include <imfilebrowser.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>

#include "utils.h"
#include "representation.h"

// Embedded font
#include "Fonts/Bahnschrift.embed"

namespace mirras
{
struct DrawTarget
{
    ImDrawList* draw_list{};
    ImVec2 cursorPos;
    float thickness{};
};

inline void initGLFW()
{
    if(!glfwInit())
        throw glfw::Error("Could not initialize GLFW");
}

inline void initGlad()
{
    if(!gladLoadGL())
        throw std::runtime_error("Could not initialize GLAD\n");
}

inline void initImGui(const glfw::Window& window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Find the imgui.ini file
    auto filePath = pathToConfigFile("imgui.ini");
    
    if(!filePath.empty()) // ... and load it if it's not empty
        ImGui::LoadIniSettingsFromDisk(filePath.string().c_str());

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    // Use another font, rather than the default one
    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    ImFont* bahnschriftFont = io.Fonts->AddFontFromMemoryCompressedTTF((void*)g_Bahnschrift, sizeof(g_Bahnschrift), 16.f, &fontConfig);
    io.FontDefault = bahnschriftFont;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

inline void shutdownImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

inline void ImGuiNewFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

inline void drawObjects(const std::vector<std::unique_ptr<Object>>& objects, const DrawTarget& drawTarget)
{
    for(const auto& obj : objects)
        obj->draw(drawTarget);
}

inline void objectsToViewportCoord(std::vector<std::unique_ptr<Object>>& objects, Vec2f wmin, Vec2f wmax, Vec2f vmin, Vec2f vmax)
{
    for(auto& obj : objects)
        obj->toViewportCoord(wmin, wmax, vmin, vmax);
}

void ImGuiFileMenu(bool& bWasFileLoaded);

void ImGuiUIForObjControl(int objIdx);

void ImGuiAddObjectPopup(bool bShowPopup);

void ImGuiListAndEditObjects();

void ImGuiUIForwindowControl();

void ImGuiMainWindow();

void renderImGui();

} // namespace mirras