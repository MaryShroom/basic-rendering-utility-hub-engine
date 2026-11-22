# Core layer: loop, window/input, math, Texture
add_library(bruh_core INTERFACE)
target_include_directories(bruh_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(bruh_core INTERFACE
    SDL2::SDL2
)

# OpenGL renderer layer: Shader, VAO/VBO/IBO, GL context/state
add_library(bruh_renderer_gl INTERFACE)
target_include_directories(bruh_renderer_gl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(bruh_renderer_gl INTERFACE
    bruh_core
    glad
)

# 2D renderer layer: OrthographicCamera, SpriteBatcher
add_library(bruh_renderer2d INTERFACE)
target_include_directories(bruh_renderer2d INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(bruh_renderer2d INTERFACE
    bruh_core
    bruh_renderer_gl
)

# Current monolithic engine + telemetry/benchmark sources
add_library(bruh_engine STATIC
    bruh.cpp
    BenchmarkRunner.cpp
    HardwareSampler.cpp
    TelemetryMonitor.cpp
    core/Application.cpp
    renderer/gl/RenderContext.cpp
)
target_include_directories(bruh_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(bruh_engine
    PUBLIC
        bruh_core
        bruh_renderer_gl
        bruh_renderer2d
        imgui
)
