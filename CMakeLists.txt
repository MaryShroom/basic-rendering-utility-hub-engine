cmake_minimum_required(VERSION 3.14)
project(BasicRenderingUtilityHub VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache enabled: ${CCACHE_PROGRAM}")
endif()

set(FETCHCONTENT_QUIET OFF CACHE BOOL "" FORCE)

# SDL2
include(FetchContent)
set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_TESTS ON)

FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.32.10
)
FetchContent_MakeAvailable(sdl2)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9
)
FetchContent_MakeAvailable(imgui)

# OpenGL
find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    thirdparty/glad/include
)

target_link_libraries(imgui PUBLIC SDL2::SDL2)

# --- Git Version Generation Setup ---
set(GIT_VERSION_FILE "${CMAKE_BINARY_DIR}/GeneratedGitVersion.hpp")

add_custom_target(UpdateGitVersion ALL
    COMMAND ${CMAKE_COMMAND}
        "-DOUTPUT_FILE=${GIT_VERSION_FILE}"
        -P "${CMAKE_SOURCE_DIR}/cmake/GetGitHash.cmake"
    DEPENDS "${CMAKE_SOURCE_DIR}/cmake/GetGitHash.cmake"
    BYPRODUCTS "${GIT_VERSION_FILE}"
    COMMENT "Checking Git revision and source status..."
    VERBATIM
)

# Main and thirdparty sources (includes generated header so CMake tracks the build dependency)
add_executable(BasicRenderingUtilityHub
    main.cpp
    engine/bruh.cpp
    engine/TelemetryMonitor.cpp
    engine/BenchmarkRunner.cpp
    engine/HardwareSampler.cpp
    thirdparty/glad/src/gl.c
)

add_dependencies(BasicRenderingUtilityHub UpdateGitVersion)

target_include_directories(BasicRenderingUtilityHub PRIVATE
    thirdparty/glad/include
    .
    "${CMAKE_BINARY_DIR}"
)

# Link SDL2, OpenGL, ImGui
target_link_libraries(BasicRenderingUtilityHub PRIVATE SDL2::SDL2 OpenGL::GL imgui)

# DEBUG flags
target_compile_options(BasicRenderingUtilityHub PRIVATE
    $<$<CONFIG:Debug>:
        -Wall
        -Wextra
        -g
        -fno-omit-frame-pointer
        -fsanitize=address,undefined
    >
)

target_link_options(BasicRenderingUtilityHub PRIVATE
    $<$<CONFIG:Debug>:
        -fsanitize=address,undefined
    >
)
