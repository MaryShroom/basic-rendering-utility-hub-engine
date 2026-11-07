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

# OpenGL
find_package(OpenGL REQUIRED)

# Main and thirdparty headers
add_executable(BasicRenderingUtilityHub
    main.cpp
    thirdparty/glad/src/glad.c
)

# Include thirdparty paths
target_include_directories(BasicRenderingUtilityHub PRIVATE
    thirdparty/glad/include
)

# Link SDL2, OpenGL
target_link_libraries(BasicRenderingUtilityHub PRIVATE SDL2::SDL2 OpenGL::GL)
