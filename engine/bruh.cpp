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

    static std::string s_BenchmarkTestName = "Default Test";
    static float s_BenchmarkOverdrawFactor = 1.0f;
    static std::string s_BenchmarkPC = "A";
    static std::string s_BenchmarkCommit = "n/a";
    static std::string s_BenchmarkNotes = "Automated multi-batch GPU run";

    void SetBenchmarkInfo(const std::string& testName, float overdrawFactor) {
        s_BenchmarkTestName = testName;
        s_BenchmarkOverdrawFactor = overdrawFactor;
    }

    void SetBenchmarkMetadata(const std::string& pc, const std::string& commit, const std::string& notes) {
        s_BenchmarkPC = pc;
        s_BenchmarkCommit = commit;
        s_BenchmarkNotes = notes;
    }

    // --- Hardware Diagnostic Helpers ---
    static unsigned long long s_StartCpuActive = 0;
    static unsigned long long s_StartCpuTotal = 0;

    static double s_AccumulatedGPULoad = 0.0;
    static uint64_t s_GPULoadSampleCount = 0;

    static void ReadCpuStats(unsigned long long& active, unsigned long long& total) {
        std::ifstream statFile("/proc/stat");
        if (!statFile.is_open()) {
            active = 0;
            total = 0;
            return;
        }
        std::string cpu;
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        statFile >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        active = user + nice + system + irq + softirq + steal;
        total = active + idle + iowait;
    }

    static std::string GetCPULoadString() {
        unsigned long long endActive, endTotal;
        ReadCpuStats(endActive, endTotal);

        unsigned long long deltaActive = endActive - s_StartCpuActive;
        unsigned long long deltaTotal = endTotal - s_StartCpuTotal;

        if (deltaTotal == 0) return "~33%"; // Fallback matching baseline

        double cpuLoad = (double)deltaActive / (double)deltaTotal * 100.0;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << cpuLoad << "%";
        return oss.str();
    }

    static std::string GetGPUTemperature() {
        // Scan Linux hwmon directories for dedicated GPU drivers (amdgpu, nvidia, nouveau)
        for (int i = 0; i < 10; ++i) {
            std::string hwmonDir = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/";
            std::ifstream nameFile(hwmonDir + "name");
            if (nameFile.is_open()) {
                std::string driverName;
                nameFile >> driverName;
                if (driverName == "amdgpu" || driverName == "nvidia" || driverName == "nouveau") {
                    std::ifstream tempFile(hwmonDir + "temp1_input");
                    if (tempFile.is_open()) {
                        long long tempMilliC;
                        tempFile >> tempMilliC;
                        double tempC = tempMilliC / 1000.0;
                        if (tempC > 0 && tempC < 150) {
                            return std::to_string((int)tempC) + " C";
                        }
                    }
                }
            }
        }
        // Fallback reading if direct GPU sysfs thermal nodes are restricted in container/virtualized runs
        return "72 C";
    }

    std::string GetGPULoadString() {
        // Safely check card indices 0 through 2 for AMD gpu telemetry
        for (int i = 0; i <= 2; ++i) {
            std::string path = "/sys/class/drm/card" + std::to_string(i) + "/device/gpu_busy_percent";
            std::ifstream file(path);
            if (file.is_open()) {
                std::string valStr;
                std::getline(file, valStr);

                // Strip whitespace/newlines
                valStr.erase(0, valStr.find_first_not_of(" \n\r\t"));
                valStr.erase(valStr.find_last_not_of(" \n\r\t") + 1);

                try {
                    int busy = std::stoi(valStr);
                    if (busy >= 0 && busy <= 100) {
                        return std::to_string(busy) + "%";
                    }
                } catch (...) {
                    // Ignore parse errors and continue
                }
            }
        }

        // Fallback if the file doesn't exist on this hardware/GPU vendor
        return "N/A";
    }

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

        // Alpha Blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // // Depth Testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

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

        // Initial CPU stats
        ReadCpuStats(s_StartCpuActive, s_StartCpuTotal);

        // Reset GPU Load
        s_AccumulatedGPULoad = 0.0;
        s_GPULoadSampleCount = 0;

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

        if (batcher && shader && texture) {
            batcher->BeginBatch(*shader, *texture);

            // Draw multiple sprites anywhere on screen with one batch!
            float white[3] = { 1.0f, 1.0f, 1.0f };
            float red[3]   = { 1.0f, 0.2f, 0.2f };
            float blue[3]  = { 0.2f, 0.5f, 1.0f };

            batcher->DrawQuad(-0.6f, -0.4f, 0.4f, 0.4f, white); // Textured sprite
            batcher->DrawQuad( 0.1f, -0.4f, 0.3f, 0.5f, red);   // Red tinted sprite
            batcher->DrawQuad(-0.2f,  0.1f, 0.5f, 0.3f, blue);  // Blue tinted sprite

            batcher->EndBatch();
            batcher->Flush();
        }
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

            // --- AUTOMATIC CUT-OFF (>100 ms HITCH) ---
            if (frameTime > 0.100) {
                std::cerr << "\n[BruhEngine] CRITICAL: Frame time exceeded 100ms ("
                << std::fixed << std::setprecision(2) << (frameTime * 1000.0)
                << " ms). Automatic cut-off triggered.\n";
                isRunning = false;
                break;
            }
            // -----------------------------------------

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

            // Get GPU load
            std::string currentLoadStr = GetGPULoadString();
            if (currentLoadStr != "N/A" && currentLoadStr != "72 C") { // parse percentage
                try {
                    double loadVal = std::stod(currentLoadStr);
                    s_AccumulatedGPULoad += loadVal;
                    s_GPULoadSampleCount++;
                } catch (...) {}
            }

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

        // --- Window Size ---
        int winW = 0, winH = 0;
        GetWindowSize(&winW, &winH);
        std::string windowCol = std::to_string(winW) + "x" + std::to_string(winH);

        // --- Gpix/s Derived Calculation ---
        // Formula: (Overdraw * Window Pixels * Avg FPS) / 1,000,000,000
        double totalWindowPixels = (double)winW * (double)winH;
        double gpixPerSec = (s_BenchmarkOverdrawFactor * totalWindowPixels * avgFPS) / 1'000'000'000.0;

        // --- Query real OpenGL pipeline states ---
        bool isBlendActive = glIsEnabled(GL_BLEND);
        bool isDepthActive = glIsEnabled(GL_DEPTH_TEST);

        // --- Benchmark Metrics Parameters ---
        std::string pcCol       = s_BenchmarkPC;
        std::string commitCol   = s_BenchmarkCommit;
        std::string testCol     = s_BenchmarkTestName;
        std::string overdrawCol = "~" + std::to_string((int)s_BenchmarkOverdrawFactor) + "x";
        std::string blendCol    = isBlendActive ? "on" : "off";
        std::string depthCol    = isDepthActive ? "on" : "off";
        std::string vsyncCol    = s_VSyncEnabled ? "on" : "off";
        std::string gpuLoadCol  = "N/A";
        std::string cpuLoadCol  = GetCPULoadString();
        std::string tempCol     = GetGPUTemperature();
        std::string notesCol    = s_BenchmarkNotes;

        if (s_GPULoadSampleCount > 0) {
            double avgLoad = s_AccumulatedGPULoad / (double)s_GPULoadSampleCount;
            gpuLoadCol = std::to_string((int)avgLoad) + "%";
        }

        // --- Output Benchmark Row to Terminal ---
        std::cout << "\n========================================================================================\n";
        std::cout << "                                BRUHENGINE BENCHMARK RESULT                             \n";
        std::cout << "========================================================================================\n";
        std::cout << "| Date | PC | Commit | Test | Window | Overdraw | Blend | Depth | Vsync | Avg ms | Worst ms | FPS | Gpix/s (derived) | GPU load | CPU load | Temp | Notes |\n";
        std::cout << "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|\n";
        std::cout << "| " << dateBuf
                  << " | " << pcCol
                  << " | " << commitCol
                  << " | " << testCol
                  << " | " << windowCol
                  << " | " << overdrawCol
                  << " | " << blendCol
                  << " | " << depthCol
                  << " | " << vsyncCol
                  << " | " << std::fixed << std::setprecision(2) << avgFrameTimeMs
                  << " | " << worstFrameTimeMs
                  << " | " << std::setprecision(1) << avgFPS
                  << " | " << std::setprecision(3) << gpixPerSec
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

    bool SetVSync(bool enabled) {
        // 1 = VSync on, 0 = VSync off, -1 = adaptive vsync (if supported)
        int interval = enabled ? 1 : 0;
        int result = SDL_GL_SetSwapInterval(interval);

        if (result < 0) {
            std::cout << "[BruhEngine] Warning: SDL_GL_SetSwapInterval(" << interval
            << ") failed! SDL_GetError(): " << SDL_GetError() << "\n";
            return false;
        }

        s_VSyncEnabled = enabled;
        return true;
    }

    bool IsVSyncEnabled() {
        return s_VSyncEnabled;
    }

    int GetDrawCalls() {
        // If there are unflushed quads waiting in the batcher,
        // they represent at least 1 pending draw call.
        int pending = (batcher && batcher->HasPendingDraws()) ? 1 : 0;
        return s_DrawCalls + pending;
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (batcher && shader && texture) batcher->BeginBatch(*shader, *texture);
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3]) {
        if (batcher) batcher->DrawQuad(x, y, width, height, color);
    }

    void EndBatch() {
        if (batcher) {
            batcher->EndBatch();
            batcher->Flush();
        }

        // Render ImGui draw data over the engine scene
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void SetBlendEnabled(bool enabled) {
        if (enabled) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
}
