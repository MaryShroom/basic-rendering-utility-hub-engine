#include "bruh.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "Texture.hpp"
#include "IndexBuffer.hpp"
#include "SpriteBatcher.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <SDL.h>
#include <glad/gl.h>
#include <iostream>
#include <functional>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>
#include <numeric>
#include <iomanip>

namespace BruhEngine {
    static SDL_Window* window = nullptr;
    static SDL_GLContext glContext = nullptr;
    static bool isRunning = false;
    static int s_DrawCalls = 0;
    static bool s_VSyncEnabled = true;
    static std::function<void(const SDL_Event&)> s_EventCallback = nullptr;

    // Engine-managed OpenGL pipeline resources
    static Shader* shader = nullptr;
    static VertexBuffer* vbo = nullptr;
    static VertexArray* vao = nullptr;
    static IndexBuffer* ibo = nullptr;
    static Texture* texture = nullptr;
    static SpriteBatcher* batcher = nullptr;

    static std::chrono::high_resolution_clock::time_point s_StartTime;
    static uint64_t s_TotalFrames = 0;
    static std::vector<float> s_FrameTimes; // In milliseconds

    // Vertex and Fragment
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "out vec3 fragColor;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "   fragColor = aColor;\n"
        "   TexCoord = aTexCoord;\n"
        "}\n";

    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 fragColor;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D u_Texture;\n"
        "void main() {\n"
        "   FragColor = texture(u_Texture, TexCoord) * vec4(fragColor, 1.0);\n"
        "}\n";

    bool Init(const char* title, int width, int height) {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
            return false;
        }

        // OpenGL 3.3 Core Profile
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // Request double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Create window
        window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

        if (!window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }

        // Create the OpenGL context
        glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            std::cerr << "OpenGL context could not be created! Ensure your hardware supports OpenGL 3.3. SDL_Error: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }

        SDL_GL_MakeCurrent(window, glContext);

        // Initialize GLAD v2
        int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
        if (version == 0) {
            std::cerr << "Failed to initialize GLAD\n";
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }

        // Vsync
        s_VSyncEnabled = true;
        if (SDL_GL_SetSwapInterval(1) < 0) {
            std::cerr << "Warning: Unable to set V-Sync! SDL_Error: " << SDL_GetError() << "\n";
        }

        // Get initial drawable size for the viewport
        int drawWidth, drawHeight;
        SDL_GL_GetDrawableSize(window, &drawWidth, &drawHeight);
        glViewport(0, 0, drawWidth, drawHeight);

        // Compile Shaders
        shader = new Shader(vertexShaderSource, fragmentShaderSource);

        // Create procedural texture
        texture = new Texture(64, 64);

        // Batcher
        batcher = new SpriteBatcher();

        // 4 Vertices: Position (X, Y), Color (R, G, B), UV (U, V)
        float quadVertices[] = {
            // Position     // Color          // UV
            -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f, // Bottom-Left
            0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Bottom-Right
            0.5f,  0.5f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-Right
            -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 1.0f  // Top-Left
        };

        unsigned int quadIndices[] = {
            0, 1, 2, // First Triangle
            2, 3, 0  // Second Triangle
        };

        vbo = new VertexBuffer(quadVertices, sizeof(quadVertices));
        ibo = new IndexBuffer(quadIndices, 6);
        vao = new VertexArray();
        vao->AddTexturedBuffer(*vbo);

        isRunning = true;
        std::cout << "BruhEngine (OpenGL) Loaded Successfully!\n";
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
        std::cout << "Shading: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

        s_StartTime = std::chrono::high_resolution_clock::now();
        s_TotalFrames = 0;
        s_FrameTimes.clear();

        // --- Setup Dear ImGui context ---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(window, glContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        return true;
    }

    void ProcessEvents() {
        // Main loop
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            // Pass event to ImGui backend first
            ImGui_ImplSDL2_ProcessEvent(&e);

            if (e.type == SDL_QUIT) {
                isRunning = false;
            }
            // Window events
            else if (e.type == SDL_WINDOWEVENT) {
                // Window resize
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    int drawWidth, drawHeight;
                    SDL_GL_GetDrawableSize(window, &drawWidth, &drawHeight);
                    glViewport(0, 0, drawWidth, drawHeight);
                }
            }
            // Foward keys to main
            if (s_EventCallback) s_EventCallback(e);
        }
    }

    void Render() {
        glClearColor(0.1f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        batcher->BeginBatch();

        // Draw multiple sprites anywhere on screen with one batch!
        float white[3] = { 1.0f, 1.0f, 1.0f };
        float red[3]   = { 1.0f, 0.2f, 0.2f };
        float blue[3]  = { 0.2f, 0.5f, 1.0f };

        batcher->DrawQuad(-0.6f, -0.4f, 0.4f, 0.4f, white); // Textured sprite
        batcher->DrawQuad( 0.1f, -0.4f, 0.3f, 0.5f, red);   // Red tinted sprite
        batcher->DrawQuad(-0.2f,  0.1f, 0.5f, 0.3f, blue);  // Blue tinted sprite

        batcher->EndBatch();
        batcher->Flush(*shader, *texture);
    }

    void Run(std::function<void(float fixedDeltaTime)> updateFunc, std::function<void()> renderFunc) {
        const double targetFPS = 60.0;
        const double fixedDt = 1.0 / targetFPS;

        Uint64 lastCounter = SDL_GetPerformanceCounter();
        double accumulator = 0.0;

        while (isRunning) {
            ProcessEvents();

            // Calculate frame time delta
            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 counterFrequency = SDL_GetPerformanceFrequency();
            double frameTime = (double)(currentCounter - lastCounter) / (double)counterFrequency;
            lastCounter = currentCounter;

            // Prevent die if a frame takes too long
            if (frameTime > 0.25) frameTime = 0.25;

            accumulator += frameTime;

            // Fixed-timestep update loop
            while (accumulator >= fixedDt) {
                if (updateFunc) updateFunc((float)fixedDt);
                accumulator -= fixedDt;
            }

            s_TotalFrames++;
            s_FrameTimes.push_back((float)(frameTime * 1000.0));

            // Reset draw call counter at the start of the frame draw phase
            s_DrawCalls = 0;

            // Render (use engine render if main.cpp did not provide one)
            if (renderFunc) renderFunc();
            else Render();

            // Buffer
            SDL_GL_SwapWindow(window);
        }
    }

    void SetWindowTitle(const std::string& title) {
        if (window) SDL_SetWindowTitle(window, title.c_str());
    }

    void GetWindowSize(int* w, int* h) {
        if (window) SDL_GetWindowSize(window, w, h);
    }

    void Shutdown() {
        // --- Calculate Performance Metrics ---
        auto endTime = std::chrono::high_resolution_clock::now();
        double totalDurationSec = std::chrono::duration<double>(endTime - s_StartTime).count();
        double avgFPS = (totalDurationSec > 0.0) ? (s_TotalFrames / totalDurationSec) : 0.0;

        float avgFrameTimeMs = 0.0f;
        float worstFrameTimeMs = 0.0f;

        if (!s_FrameTimes.empty()) {
            double sum = 0.0;
            for (float ft : s_FrameTimes) {
                sum += ft;
                if (ft > worstFrameTimeMs) worstFrameTimeMs = ft;
            }
            avgFrameTimeMs = (float)(sum / s_FrameTimes.size());
        }

        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tmBuffer;
#if defined(_WIN32)
        localtime_s(&tmBuffer, &now);
#else
        localtime_r(&now, &tmBuffer);
#endif
        char dateBuf[32];
        std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &tmBuffer);

        // --- Benchmark Metrics Parameters ---
        std::string pcCol       = "A";
        std::string commitCol   = "n/a";
        std::string testCol     = "100k Sprites (1.0x)";
        std::string overdrawCol = "~25000x";
        std::string blendCol    = "on";
        std::string depthCol    = "on";
        std::string vsyncCol    = s_VSyncEnabled ? "on" : "off";
        std::string gpuLoadCol  = "n/a";
        std::string cpuLoadCol  = "n/a";
        std::string tempCol     = "n/a";
        std::string notesCol    = "Automated shutdown baseline run";

        // --- Output Benchmark Row to Terminal ---
        std::cout << "\n========================================================================================\n";
        std::cout << "                                BRUHENGINE BENCHMARK RESULT                             \n";
        std::cout << "========================================================================================\n";
        std::cout << "| Date | PC | Commit | Test | Overdraw | Blend | Depth | Vsync | Avg ms | Worst ms | FPS | GPU load | CPU load | Temp | Notes |\n";
        std::cout << "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|\n";
        std::cout << "| " << dateBuf
                  << " | " << pcCol
                  << " | " << commitCol
                  << " | " << testCol
                  << " | " << overdrawCol
                  << " | " << blendCol
                  << " | " << depthCol
                  << " | " << vsyncCol
                  << " | " << std::fixed << std::setprecision(2) << avgFrameTimeMs
                  << " | " << worstFrameTimeMs
                  << " | " << std::setprecision(1) << avgFPS
                  << " | " << gpuLoadCol
                  << " | " << cpuLoadCol
                  << " | " << tempCol
                  << " | " << notesCol << " |\n";
        std::cout << "========================================================================================\n\n";

        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        // GPU resources cleanup
        delete shader;
        delete vbo;
        delete ibo;
        delete vao;
        delete texture;
        delete batcher;

        shader = nullptr;
        vbo = nullptr;
        ibo = nullptr;
        vao = nullptr;
        texture = nullptr;
        batcher = nullptr;

        if (glContext) SDL_GL_DeleteContext(glContext);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        std::cout << "BruhEngine Shut Down Cleanly.\n";
    }

    void SetVSync(bool enabled) {
        s_VSyncEnabled = enabled;
        // 1 for VSync on, 0 for VSync off (unlocked fps)
        if (SDL_GL_SetSwapInterval(enabled ? 1 : 0) < 0) {
            std::cerr << "Warning: Unable to modify V-Sync! SDL_Error: " << SDL_GetError() << "\n";
        } else {
            std::cout << "V-Sync " << (enabled ? "Enabled" : "Disabled") << "\n";
        }
    }

    bool IsVSyncEnabled() {
        return s_VSyncEnabled;
    }

    int GetDrawCalls() {
        return s_DrawCalls;
    }

    void IncrementDrawCalls() {
        s_DrawCalls++;
    }

    void SetEventCallback(std::function<void(const SDL_Event&)> callback) {
        s_EventCallback = callback;
    }

    void BeginBatch() {
        glClearColor(0.1f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        s_DrawCalls = 0;

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (batcher) batcher->BeginBatch();
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3]) {
        if (batcher) batcher->DrawQuad(x, y, width, height, color);
    }

    void EndBatch() {
        if (batcher) {
            batcher->EndBatch();
            batcher->Flush(*shader, *texture);
        }

        // Render ImGui draw data over the engine scene
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
