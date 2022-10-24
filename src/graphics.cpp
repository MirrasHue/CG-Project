#include "graphics.h"

#include <string>

namespace mirras
{
void ImGuiSaveFilePopup(const char* str_id)
{
    ImGui::SetNextWindowSize(ImVec2{300, 100});
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});

    if(ImGui::BeginPopup(str_id))
    {
        ImGui::Text("Enter the file name");
        static char buf[64];
        bool bEnterPressed = false;

        bEnterPressed = ImGui::InputText(".xml", buf, 64, ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::Spacing();

        if(ImGui::Button("Save") || bEnterPressed)
        {
            std::string fileName = buf;

            if(fileName.empty())
                g_Logger.AddLog("Empty file name, unable to save\n");
            else
            {
                if(saveXMLFile((fileName + ".xml").c_str()))
                    g_Logger.AddLog("Successfully saved file to %s\n", fs::current_path().string().c_str());
                else
                    g_Logger.AddLog("Failed to save file\n");

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        
        if(ImGui::Button("Cancel"))
        {
            buf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void ImGuiFileMenu(bool& bWasFileLoaded)
{
    static ImGui::FileBrowser fileBrowser(ImGuiFileBrowserFlags_NoModal);

    bool bSaveFile = false;

    if(ImGui::BeginMenu("File"))
    {
        if(ImGui::MenuItem("Load File"))
        {
            fileBrowser.SetTypeFilters({".xml"});
            fileBrowser.Open();
        }

        if(bWasFileLoaded)
        {
            if(ImGui::MenuItem("Save File"))
                bSaveFile = true;

            if(ImGui::MenuItem("Output File"))
            {
                g_Logger.AddLog("Outputting objects' viewport coordinates...\n");

                std::ofstream outputFile{"output.txt"};
                
                outputFile << "Viewport size: " << g_Viewport.width << " x " << g_Viewport.height << "\n\n";
                outputFile << "Object:   X     Y\n\n";

                writeObjectsVpCoordToFile(outputFile);
                g_Logger.AddLog("Done! Coordinates have been written to output.txt\n");
            }
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

    if(bSaveFile)
        ImGui::OpenPopup("Save File");

    ImGuiSaveFilePopup("Save File");
}

void ImGuiUIForObjControl(int objIdx)
{
    ImGui::Text("Controls");

    float currentCursorPosX = ImGui::GetCursorPosX();

    static glm::mat4 transform{1.f};
    static float translationX{}, translationY{}; // Just for visualization
    
    // Calculate the transformations as they are requested, so that when we apply them, the order will be preserved

    if(ImGuiAlignedButton(ButtonType::Arrow, "up", 0.5f, ImGuiDir_Up))
    {
        translationY += translationStep;
        transform = glm::translate(transform, glm::vec3(0.f, translationStep, 0.f));
    }

    if(ImGuiAlignedButton(ButtonType::Arrow, "left", 0.25f, ImGuiDir_Left))
    {
        translationX -= translationStep;
        transform = glm::translate(transform, glm::vec3(-translationStep, 0.f, 0.f));
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(currentCursorPosX);

    if(ImGuiAlignedButton(ButtonType::Arrow, "right", 0.75f, ImGuiDir_Right))
    {
        translationX += translationStep;
        transform = glm::translate(transform, glm::vec3(translationStep, 0.f, 0.f));
    }

    if(ImGuiAlignedButton(ButtonType::Arrow, "down", 0.5f, ImGuiDir_Down))
    {
        translationY -= translationStep;
        transform = glm::translate(transform, glm::vec3(0.f, -translationStep, 0.f));
    }
    
    static float angle{};
    static Vec2f center{};

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 buttonSize = {ImGui::GetContentRegionAvail().x * 0.5f - style.FramePadding.x, 0.f};
    
    if(ImGui::Button("Rotate CCW", buttonSize))
    {
        angle += angleStep;

        center = g_World.objects[objIdx]->getCenter();
        transform = transform * rotateAroundCenter(center, angleStep);
    }

    ImGui::SameLine();

    if(ImGui::Button("Rotate CW", ImVec2{-FLT_MIN, 0.f}))
    {
        angle -= angleStep;

        center = g_World.objects[objIdx]->getCenter();
        transform = transform * rotateAroundCenter(center, -angleStep);
    }

    static float scaleFactor{1.f};

    if(ImGui::Button("Scale +", buttonSize))
    {
        scaleFactor *= 1.f + scaleFactorStep;

        center = g_World.objects[objIdx]->getCenter();
        transform = transform * scaleAroundCenter(center, 1 + scaleFactorStep);
    }

    ImGui::SameLine();

    if(ImGui::Button("Scale -", ImVec2{-FLT_MIN, 0.f}))
    {
        scaleFactor *= 1.f / (1.f + scaleFactorStep);

        center = g_World.objects[objIdx]->getCenter();
        transform = transform * scaleAroundCenter(center, 1 / (1 + scaleFactorStep));
    }

    ImGui::Separator();

    // Show transformations
    ImGui::Text("Translation: x=%.2f | y=%.2f", translationX, translationY);
    ImGui::Text("Rotation: %.2f°", angle);
    ImGui::Text("Scale: %.2fx", scaleFactor);

    ImGui::Separator();

    if(ImGui::Button("Apply", buttonSize))
    {
        g_World.objects[objIdx]->applyTransform(transform);
    }

    ImGui::SameLine();

    if(ImGui::Button("Reset Transform", ImVec2{-FLT_MIN, 0.f}))
    {
        translationX = 0.f;
        translationY = 0.f;
        angle = 0.f;
        scaleFactor = 1.f;
        transform = glm::mat4(1.f);
    }
}

void ImGuiAddObjectPopup(const char* str_id)
{
    ImGui::SetNextWindowSize(ImVec2{300, 200});
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});

    if(ImGui::BeginPopup(str_id))
    {
        static std::vector<Point> newObjPoints{Point{}};

        ImGui::Text("Enter X and Y coordinates");
        if(ImGui::BeginListBox("##listNewPoints", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y * 0.8f)))
        {
            for(size_t i = 0; i < newObjPoints.size(); ++i)
            {
                ImGui::PushID(i);
                ImGui::SetNextItemWidth(100.f);
                ImGui::InputFloat("##x", &newObjPoints[i].x, 0.f, 0.f, "%.2f");
                
                ImGui::SameLine();

                ImGui::SetNextItemWidth(100.f);
                ImGui::InputFloat("##y", &newObjPoints[i].y, 0.f, 0.f, "%.2f");
                ImGui::PopID();
            }

            ImGui::SameLine();
            if(ImGui::Button(" + "))
                newObjPoints.emplace_back(Point{});

            ImGui::EndListBox();
        }

        ImGui::Spacing();

        if(ImGui::Button("Add"))
        {
            if(newObjPoints.size() == 1)
            {
                g_World.objects.emplace_back(std::make_unique<Point>(newObjPoints[0]));
            }
            else if(newObjPoints.size() == 2)
            {
                LineSegment line;
                line.p0 = newObjPoints[0];
                line.p1 = newObjPoints[1];
                g_World.objects.emplace_back(std::make_unique<LineSegment>(line));
            }
            else if(newObjPoints.size() > 2)
            {
                g_World.objects.emplace_back(std::make_unique<Polygon>(std::move(newObjPoints)));
            }
            else
                g_Logger.AddLog("Not possible to add object with 0 points\n");

            newObjPoints.clear();
            newObjPoints.emplace_back(Point{});
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel"))
        {
            newObjPoints.clear();
            newObjPoints.emplace_back(Point{});
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ImGuiListAndEditObjects()
{
    ImGui::Begin("Objects");
    {
        if(ImGui::Button("Add Object", ImVec2{-FLT_MIN, 0.f}))
            ImGui::OpenPopup("Add Object");

        ImGuiAddObjectPopup("Add Object");

        static int selectedIdx = -1;

        if(ImGui::BeginListBox("##selectObj", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y * 0.5f)))
        {
            for(int i = 0; i < (int)g_World.objects.size(); ++i)
            {
                bool bIsSelected = (selectedIdx == i);

                if(ImGui::Selectable((g_World.objects[i]->getTypeName() + std::to_string(i)).c_str(), bIsSelected, ImGuiSelectableFlags_AllowItemOverlap))
                {
                    if(bIsSelected) // Clicking on the same object twice to deselect
                    {
                        selectedIdx = -1;
                        bIsSelected = false;
                    }
                    else
                    {
                        selectedIdx = i;
                        bIsSelected = true;
                    }
                }

                if(bIsSelected)
                    g_World.objects[i]->bIsSelected = true;
                else
                    g_World.objects[i]->bIsSelected = false;

                ImGui::SameLine();

                ImGui::PushID(i);
                if(ImGuiAlignedButton(ButtonType::Small, "delete", 1.f))
                {
                    g_World.objects.erase(g_World.objects.begin() + i); // Moves one pos back the elements after the erased one
                    //std::erase(g_World.objects, g_World.objects[i]); // Possibly more expensive due to comparisons
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        ImGui::Separator();

        // Second check for the case when some object was already selected and another file with less objects is loaded
        if(selectedIdx == -1 || selectedIdx > (int) g_World.objects.size() - 1)
        {
            ImGui::End();
            return;
        }

        ImGuiUIForObjControl(selectedIdx);
    }
    ImGui::End();
}

void ImGuiUIForwindowControl()
{
    ImGui::Text("Window Controls");

    float currentCursorPosX = ImGui::GetCursorPosX();

    if(ImGuiAlignedButton(ButtonType::Arrow, "up", 0.5f, ImGuiDir_Up))
    {
        g_Window.wmin.y += translationStep;
        g_Window.wmax.y += translationStep;
    }

    if(ImGuiAlignedButton(ButtonType::Arrow, "left", 0.25f, ImGuiDir_Left))
    {
        g_Window.wmin.x -= translationStep;
        g_Window.wmax.x -= translationStep;
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(currentCursorPosX);

    if(ImGuiAlignedButton(ButtonType::Arrow, "right", 0.75f, ImGuiDir_Right))
    {
        g_Window.wmin.x += translationStep;
        g_Window.wmax.x += translationStep;
    }

    if(ImGuiAlignedButton(ButtonType::Arrow, "down", 0.5f, ImGuiDir_Down))
    {
        g_Window.wmin.y -= translationStep;
        g_Window.wmax.y -= translationStep;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 buttonSize = {ImGui::GetContentRegionAvail().x * 0.5f - style.FramePadding.x, 0.f};

    if(ImGui::Button("Rotate CCW", buttonSize))
        rotateWindow(angleStep);

    ImGui::SameLine();

    if(ImGui::Button("Rotate CW", ImVec2{-FLT_MIN, 0.f}))
        rotateWindow(-angleStep);


    if(ImGui::Button("Zoom In", buttonSize))
        scaleWindow(1.f / (1.f + scaleFactorStep));

    ImGui::SameLine();

    if(ImGui::Button("Zoom Out", ImVec2{-FLT_MIN, 0.f}))
        scaleWindow(1.f + scaleFactorStep);

    if(ImGui::Button("Reset Window", ImVec2{-FLT_MIN, 0.f}))
    {
        /*
          I realized it's not possible to reset the window (all the time) in the
          way I'm doing it now, translations with rotations are not commutative.
          One option is to store the initial pos of each point, maybe not so elegant
        */
        resetWindow();
    }
}

void ImGuiMainWindow()
{
    static bool bWasFileLoaded = false;
    static float thickness = 1.5f;
    
    uint32_t color{};

    if(ImGui::BeginMainMenuBar())
    {
        ImGuiFileMenu(bWasFileLoaded);      

        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Panel", nullptr, ImGuiWindowFlags_NoTitleBar); ImGui::End();

    g_Logger.Draw("Log");

    if(!bWasFileLoaded)
        return;

    ImGuiListAndEditObjects();

    ImGui::Begin("Panel", nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        static ImVec4 RGBAPoint = {0.f, 1.f, 0.9f, 1.f};
        static ImVec4 RGBALine  = {0.9f, 1.f, 0.f, 1.f};
        static ImVec4 RGBAPoly  = {0.f, 1.f, 0.1f, 1.f};

        ImGui::ColorEdit4("Point", RGBAPoint.firstElemAddr(), ImGuiColorEditFlags_PickerHueWheel);
        Point::color = ImColor(RGBAPoint);

        ImGui::Separator();

        ImGui::ColorEdit4("Line", RGBALine.firstElemAddr(), ImGuiColorEditFlags_PickerHueWheel);
        LineSegment::color = ImColor(RGBALine);

        ImGui::Separator();
        
        ImGui::ColorEdit4("Polygon", RGBAPoly.firstElemAddr(), ImGuiColorEditFlags_PickerHueWheel);
        Polygon::color = ImColor(RGBAPoly);

        ImGui::Separator();

        ImGui::DragFloat("Thickness", &thickness, 0.05f, 1.5f, 5.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();

        ImGui::Text("Step for each transformation");
        ImGui::SameLine();
        ImGuiHelpMarker("Used for both window and objects");

        ImGui::DragFloat("Translation", &translationStep, 0.5f, 0.5f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Angle", &angleStep, 0.5f, 1.f, 359.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Scale", &scaleFactorStep, 0.005f, 0.05f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();

        ImGuiUIForwindowControl();
    }
    ImGui::End();

    // Account for the border on all sides
    // Maybe add a flag to signal when another file is loaded, so that we update these 2 values accordingly
    static float totalWidth  = 2 * g_Viewport.borderW + g_Viewport.width;
    static float totalHeight = 2 * g_Viewport.borderH + g_Viewport.height;

    ImGui::SetNextWindowSize({totalWidth, totalHeight});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 currentDrawPos = ImGui::GetCursorScreenPos();

        DrawTarget drawTarget{.draw_list = draw_list,
                              .cursorPos = currentDrawPos,
                              .thickness = thickness};

        if(ImGui::IsWindowDocked())
        {
            auto[totalW, totalH] = ImGui::GetContentRegionAvail();
            g_Viewport.width  = totalW - 2 * g_Viewport.borderW;
            g_Viewport.height = totalH - 2 * g_Viewport.borderH;
        }
        else
        {
            g_Viewport.width  = totalWidth - 2 * g_Viewport.borderW;
            g_Viewport.height = totalHeight - 2 * g_Viewport.borderH;
        }

        Vec2f vmin = {g_Viewport.borderW, g_Viewport.borderH};
        Vec2f vmax = {g_Viewport.width, g_Viewport.height};
        objectsToViewportCoord(g_World.objects, g_Window.wmin, g_Window.wmax, vmin, vmax);
        
        drawObjects(g_World.objects, drawTarget);

        // Draw viewport borders
        Vec2f borderMin = {g_Viewport.borderW, g_Viewport.borderH};
        Vec2f borderMax = {g_Viewport.width + g_Viewport.borderW, g_Viewport.height + g_Viewport.borderH};
        draw_list->AddRect(borderMin + currentDrawPos, borderMax + currentDrawPos, IM_COL32_WHITE, 1.f, ImDrawFlags_None, 0.5f);
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
