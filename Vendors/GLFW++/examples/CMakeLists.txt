set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Target to build all examples and tests at once
add_custom_target(glfwpp_all_examples)

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
	add_executable(glfwpp_example_basic basic.cpp)
	target_link_libraries(glfwpp_example_basic PRIVATE libglew_static GLFWPP)
	target_compile_definitions(glfwpp_example_basic PRIVATE GLEW_STATIC)

	add_executable(glfwpp_example_events events.cpp)
	target_link_libraries(glfwpp_example_events PRIVATE libglew_static GLFWPP)
	target_compile_definitions(glfwpp_example_events PRIVATE GLEW_STATIC)

	add_executable(glfwpp_example_window_attributes window_attributes.cpp)
	target_link_libraries(glfwpp_example_window_attributes PRIVATE libglew_static GLFWPP)
	target_compile_definitions(glfwpp_example_window_attributes PRIVATE GLEW_STATIC)

	add_executable(glfwpp_example_imgui_integration imgui_integration.cpp)
	target_link_libraries(glfwpp_example_imgui_integration PRIVATE libglew_static GLFWPP Imgui)
	target_compile_definitions(glfwpp_example_imgui_integration PRIVATE GLEW_STATIC)

	add_executable(glfwpp_example_multiple_windows multiple_windows.cpp)
	target_link_libraries(glfwpp_example_multiple_windows PRIVATE libglew_static GLFWPP)
	target_compile_definitions(glfwpp_example_multiple_windows PRIVATE GLEW_STATIC)

	add_executable(glfwpp_test_multiple_tus multiple_tus1.cpp multiple_tus2.cpp)
	target_link_libraries(glfwpp_test_multiple_tus PRIVATE libglew_static GLFWPP)
	target_compile_definitions(glfwpp_test_multiple_tus PRIVATE GLEW_STATIC)

	add_executable(glfwpp_test_type_traits type_traits.cpp)
	target_link_libraries(glfwpp_test_type_traits PRIVATE libglew_static GLFWPP)
	target_compile_definitions(glfwpp_test_type_traits PRIVATE GLEW_STATIC)

	add_dependencies(glfwpp_all_examples
		glfwpp_example_basic
		glfwpp_example_events
		glfwpp_example_window_attributes
		glfwpp_example_imgui_integration
		glfwpp_example_multiple_windows
		glfwpp_test_multiple_tus
		glfwpp_test_type_traits
	)
else()
	add_executable(glfwpp_example_emscripten emscripten.cpp)
	target_link_libraries(glfwpp_example_emscripten PRIVATE GLFWPP Imgui)

	add_dependencies(glfwpp_all_examples
		glfwpp_example_emscripten
	)
endif()
