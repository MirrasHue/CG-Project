cmake_minimum_required(VERSION 3.20)
project(CG_Project)

set(CMAKE_CXX_STANDARD 20)

# GLFW Flags
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# GLM Flag
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# Fetch source files
file(GLOB_RECURSE src_project CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE src_glad CONFIGURE_DEPENDS Vendors/Glad/src/*.c)
file(GLOB_RECURSE src_imgui CONFIGURE_DEPENDS Vendors/ImGui/src/*.cpp)
file(GLOB_RECURSE src_pugixml CONFIGURE_DEPENDS Vendors/PugiXML/src/*.cpp)
file(GLOB_RECURSE src_imgui_filebrowser CONFIGURE_DEPENDS Vendors/ImGui-Filebrowser/*.cpp)

add_subdirectory(Vendors/GLFW)
add_subdirectory(Vendors/GLM)

add_executable(CG_Project ${src_project} ${src_glad} ${src_imgui} ${src_pugixml} ${src_imgui_filebrowser})

target_link_libraries(CG_Project glfw glm)

include_directories(Vendors/GLAD/include)
include_directories(Vendors/GLFW++/include)
include_directories(Vendors/ImGui/src)
include_directories(Vendors/PugiXML/src)
include_directories(Vendors/ImGui-Filebrowser)
include_directories(src)

# Disable console window
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_options(CG_Project PRIVATE -mwindows)
endif()