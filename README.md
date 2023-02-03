Instructions to compile the project:

1. Make sure your compiler supports C++20 or newer.
   For Windows, you can download the latest GCC release from https://winlibs.com/#download-release

2. Download and install CMake from https://cmake.org/download/

3. Open the terminal on this directory and run the following commands:

    mkdir build \
    cd build \
    cmake ..

4. After the configuration is done, compile with (debug build):

    cmake --build .

... or for the release build:

    cmake --build . --config Release

5. Find and run the generated executable

6. On the first use, you may have to adjust the window layout if imgui.ini is not in your root directory
