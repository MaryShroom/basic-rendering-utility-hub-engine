# Core layer: loop, window/input, math, Texture
add_library(bruh_core STATIC
    core/Application.cpp
)
target_include_directories(bruh_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(bruh_core PUBLIC
    SDL2::SDL2
    imgui
)

# OpenGL renderer layer: Shader, VAO/VBO/IBO, GL context/state
add_library(bruh_renderer_gl STATIC
    renderer/gl/RenderContext.cpp
)
target_include_directories(bruh_renderer_gl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(bruh_renderer_gl PUBLIC
    bruh_core
    glad
    OpenGL::GL
)

# 2D renderer layer: OrthographicCamera, SpriteBatcher
add_library(bruh_renderer2d STATIC
    renderer2d/Renderer2D.cpp
)
target_include_directories(bruh_renderer2d PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(bruh_renderer2d PUBLIC
    bruh_core
    bruh_renderer_gl
)

# Current monolithic engine + telemetry/benchmark sources
add_library(bruh_engine STATIC
    bruh.cpp
    BenchmarkRunner.cpp
    HardwareSampler.cpp
    TelemetryMonitor.cpp
)
target_include_directories(bruh_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(bruh_engine PUBLIC
    bruh_core
    bruh_renderer_gl
    bruh_renderer2d
    imgui
)
