#include "graphics.h"


namespace mirras
{
void ImGuiFileMenu(bool& bWasFileLoaded)
{
    static ImGui::FileBrowser fileBrowser(ImGuiFileBrowserFlags_NoModal);

    if(ImGui::BeginMenu("File"))
    {
        if(ImGui::MenuItem("Load File"))
        {
            fileBrowser.SetTypeFilters({".xml"});
            fileBrowser.Open();
        }

        if(bWasFileLoaded)
            if(ImGui::MenuItem("Output File"))
            {
                g_Logger.AddLog("Outputting objects' viewport coordinates...\n");

                std::ofstream outputFile{"output.txt"};
                
                outputFile << "Viewport size: " << g_Viewport.width << " x " << g_Viewport.height << "\n\n";
                outputFile << "Object:   X     Y\n\n";

                writeObjectsVpCoordToFile(outputFile);
                g_Logger.AddLog("Done! Coordinates have been written to output.txt\n");
            }

        ImGui::EndMenu();
    }

    fileBrowser.Display();

    if(fileBrowser.HasSelected())
    {
        std::string filePath = fileBrowser.GetSelected().string();
        std::ranges::replace(filePath, '\\', '/');

        g_Logger.AddLog("Selected file path: %s\n", filePath.c_str());

        if(auto data = loadDataFromXMLFile(filePath.c_str()))
        {
            bWasFileLoaded = true;

            g_World = std::move(data->world);
            g_Window = data->window;
            g_Viewport = data->viewport;
        }

        fileBrowser.ClearSelected();
    }
}

void ImGuiMainWindow()
{
    static bool bWasFileLoaded = false;
    static ImVec4 RGBA = {0.f, 0.7f, 1.f, 1.f};
    static float thickness = 1.5f;
    uint32_t color{};

    ImGui::Begin("Panel", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);
    {
        if(ImGui::BeginMenuBar())
        {
            ImGuiFileMenu(bWasFileLoaded);      

            ImGui::EndMenuBar();
        }

        if(bWasFileLoaded)
        {
            ImGui::ColorEdit4("Color", RGBA.firstElemAddr());
            color = ImColor(RGBA);

            ImGui::DragFloat("Thickness", &thickness, 0.05f, 1.5f, 5.f, "%.02f");
        }
    }
    ImGui::End();

    g_Logger.Draw("Log");

    if(!bWasFileLoaded)
        return;

    // Account for the border on all sides
    static const float initialWidth  = 2 * g_Viewport.borderW + g_Viewport.width;
    static const float initialHeight = 2 * g_Viewport.borderH + g_Viewport.height;

    ImGui::SetNextWindowSize({initialWidth, initialHeight});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 currentDrawPos = ImGui::GetCursorScreenPos();

        DrawTarget drawTarget{.draw_list = draw_list,
                              .cursorPos = currentDrawPos,
                              .color = color,
                              .thickness = thickness};

        if(ImGui::IsWindowDocked())
        {
            auto[totalWidth, totalHeight] = ImGui::GetContentRegionAvail();
            g_Viewport.width  = totalWidth - 2 * g_Viewport.borderW;
            g_Viewport.height = totalHeight - 2 * g_Viewport.borderW;
        }
        else
        {
            g_Viewport.width  = initialWidth - 2 * g_Viewport.borderW;
            g_Viewport.height = initialHeight - 2 * g_Viewport.borderW;
        }

        Vec2f vmin = {g_Viewport.borderW, g_Viewport.borderH};
        Vec2f vmax = {g_Viewport.width, g_Viewport.height};
        objectsToViewportCoord(g_World.objects, g_Window.wmin, g_Window.wmax, vmin, vmax);
        
        drawObjects(g_World.objects, drawTarget);

        // Draw viewport borders
        static ImVec4 borderColor = ImVec4(1.f, 1.f, 1.f, 1.f);
        Vec2f borderMin = {g_Viewport.borderW, g_Viewport.borderH};
        Vec2f borderMax = {g_Viewport.width + g_Viewport.borderW, g_Viewport.height + g_Viewport.borderH};
        draw_list->AddRect(borderMin + currentDrawPos, borderMax + currentDrawPos, ImColor(borderColor), 1.f, ImDrawFlags_None, 0.5f);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void renderImGui()
{
    ImGuiNewFrame();

    ImGui::DockSpaceOverViewport();

    ImGuiMainWindow();

    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        glfw::Window& backup_current_context = glfw::getCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfw::makeContextCurrent(backup_current_context);
    }
}

} // namespace mirras
