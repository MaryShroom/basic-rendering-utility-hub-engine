#include "engine/bruh.hpp"
#include "engine/TelemetryMonitor.hpp"
#include "engine/BenchmarkRunner.hpp"

#if __has_include("GeneratedGitVersion.hpp")
#include "GeneratedGitVersion.hpp"
#endif

#include "imgui.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <SDL.h>

#ifndef COMMIT_HASH
#define COMMIT_HASH "local-dev"
#endif

using namespace BruhEngine;

void PrintHelp() {
    std::cout << "BruhEngine Benchmark Suite\n"
              << "Usage: BruhEngineBenchmark [options]\n\n"
              << "Options:\n"
              << "  --window <w> <h>   Window width and height (default: 700 700)\n"
              << "  --count <int>     Particle / sprite counts (default: 10000)\n"
              << "  --pixels <int>     Sprite size width/height in pixels (default: 35)\n"
              << "  --blend <0|1>      Enable alpha blending (default: 0)\n"
              << "  --depth <0|1>      Enable depth testing (default: 0)\n"
              << "  --vsync <0|1>      Enable VSync (default: 0)\n"
              << "  --telemetry <0|1>  Initial telemetry HUD visibility (default: 0)\n"
              << "  --duration <sec>   Active benchmark run duration in seconds (default: 60.0)\n"
              << "  --warmup <sec>     Warmup grace period in seconds (default: 2.0)\n"
              << "  --benchmark <0|1>  Enable automated benchmark runner (default: 1)\n"
              << "  -h, --help         Display this help message\n";
}

struct Particle {
    float x, y;
    float vx, vy;
    float color[3];
};

int main(int argc, char* argv[]) {
    // Default Configuration Value
    int winW = 700, winH = 700;
    int particleCount = 10000;       // 2000 (5x), 4000 (10x) or 8000 (20x)
    int spritePixelWH = 35;          // Pixel size 35px = 0.1 NDC size | For now, width and height sprite size is same, in px
    bool enableBenchmarkMode = true; // Benchmark and Telemetry Opt-In
    bool blendEnabled = false;
    bool depthEnabled = false;
    bool vsyncEnabled = false;
    bool telemetryVisible = false;
    double benchmarkDuration = 60.0;
    double warmupTimer = 2.0f;

    // Parse Command-Line Arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--window" && i + 2 < argc) {
            winW = std::stoi(argv[++i]);
            winH = std::stoi(argv[++i]);
        } else if ((arg == "--count" || arg == "-c") && i + 1 < argc) {
            particleCount = std::stoi(argv[++i]);
        } else if ((arg == "--pixels" || arg == "-p") && i + 1 < argc) {
            spritePixelWH = std::stoi(argv[++i]);
        } else if ((arg == "--blend" || arg == "-b") && i + 1 < argc) {
            blendEnabled = (std::stoi(argv[++i]) != 0);
        } else if ((arg == "--depth" || arg == "-d") && i + 1 < argc) {
            depthEnabled = (std::stoi(argv[++i]) != 0);
        } else if ((arg == "--vsync" || arg == "-v") && i + 1 < argc) {
            vsyncEnabled = (std::stoi(argv[++i]) != 0);
        } else if (arg == "--telemetry" && i + 1 < argc) {
            telemetryVisible = (std::stoi(argv[++i]) != 0);
        } else if (arg == "--duration" && i + 1 < argc) {
            benchmarkDuration = std::stod(argv[++i]);
        } else if (arg == "--warmup" && i + 1 < argc) {
            warmupTimer = std::stod(argv[++i]);
        } else if (arg == "--benchmark" && i + 1 < argc) {
            enableBenchmarkMode = (std::stoi(argv[++i]) != 0);
        } else if (arg == "--help" || arg == "-h") {
            PrintHelp();
            return 0;
        }
    }

    if (!Init("BruhEngine - Stress Test", winW, winH)) {
        return -1;
    }

    int pixelWidth = winW, pixelHeight = winH;
    BruhEngine::GetDrawableSize(&pixelWidth, &pixelHeight);

    // Particles size (converted pixel target into proper NDC scale)
    float spriteWidth = (float)spritePixelWH / (float)pixelWidth * 2.0f;
    float spriteHeight = (float)spritePixelWH / (float)pixelHeight * 2.0f;

    // Initialize however many moving particles
    std::vector<Particle> particles(particleCount);
    for (int i = 0; i < particleCount; ++i) {
        particles[i].x = ((float)(rand() % 2000) / 1000.0f) - 1.0f; // Random X (-1.0 to 1.0)
        particles[i].y = ((float)(rand() % 2000) / 1000.0f) - 1.0f; // Random Y (-1.0 to 1.0)
        particles[i].vx = ((float)(rand() % 100) - 50.0f) / 300.0f;  // Velocity X
        particles[i].vy = ((float)(rand() % 100) - 50.0f) / 300.0f;  // Velocity Y

        particles[i].color[0] = (float)(rand() % 100) / 100.0f;     // R tint
        particles[i].color[1] = (float)(rand() % 100) / 100.0f;     // G tint
        particles[i].color[2] = 1.0f;                               // B tint
    }

    // Compute Precise Clipped Overdraw Factor Once at Startup
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    float xMin = -spriteWidth * pivotX;
    float xMax = spriteWidth * (1.0f - pivotX);
    float yMin = -spriteHeight * pivotY;
    float yMax = spriteHeight * (1.0f - pivotY);

    double totalVisibleArea = 0.0;
    for (const auto& p : particles) {
        float left   = p.x + xMin;
        float right  = p.x + xMax;
        float bottom = p.y + yMin;
        float top    = p.y + yMax;

        // Clamp edges to NDC viewport range [-1.0, 1.0]
        float cLeft   = std::max(-1.0f, std::min(1.0f, left));
        float cRight  = std::max(-1.0f, std::min(1.0f, right));
        float cBottom = std::max(-1.0f, std::min(1.0f, bottom));
        float cTop    = std::max(-1.0f, std::min(1.0f, top));

        float cWidth  = cRight - cLeft;
        float cHeight = cTop - cBottom;

        if (cWidth > 0.0f && cHeight > 0.0f) {
            totalVisibleArea += (double)(cWidth * cHeight);
        }
    }
    double overdrawFactor = totalVisibleArea / 4.0;

    TelemetryMonitor telemetry;
    telemetry.Init();
    telemetry.SetVisible(telemetryVisible);

    BenchmarkRunner benchmark;
    if (enableBenchmarkMode) {
        std::string testConfigName = std::to_string(particleCount) +
        " Sprites | " + std::to_string(spritePixelWH) + "px | Pivot(0.5,0.5) | Commit:" + std::string(COMMIT_HASH);

        benchmark.Configure(true, testConfigName, overdrawFactor, warmupTimer, benchmarkDuration);
    }

    BruhEngine::SetBlendEnabled(blendEnabled); // Toggle blend for fill-rate testing
    SetDepthEnabled(depthEnabled); // Depth Toggle
    SetVSync(vsyncEnabled); // Toggle V-Sync

    // Register event listener to toggle VSync on pressing 'V'
    SetEventCallback([&telemetry](const SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_v) {
                SetVSync(!IsVSyncEnabled());
            } else if (e.key.keysym.sym == SDLK_h) {
                telemetry.ToggleVisible();
            } else if (e.key.keysym.sym == SDLK_d) {
                SetDepthEnabled(!IsDepthEnabled());
            }
        }
    });

    // Fixed update: move particles and bounce off screen edges
    auto updateLogic = [&](float fixedDt) {
        for (auto& p : particles) {
            p.x += p.vx * fixedDt;
            p.y += p.vy * fixedDt;

            // Screen boundary bounce (NDC range roughly -0.95 to 0.95)
            if (p.x < -0.95f || p.x > 0.95f) p.vx = -p.vx;
            if (p.y < -0.95f || p.y > 0.95f) p.vy = -p.vy;
        }
    };

    // Keep track of time for precise per-frame delta in main.cpp
    Uint64 lastCounter = SDL_GetPerformanceCounter();

    // Custom render logic: what to draw each frame
    auto renderLogic = [&]() {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        double frameTime = (double)(currentCounter - lastCounter) / (double)SDL_GetPerformanceFrequency();
        lastCounter = currentCounter;

        if (frameTime > 0.25) frameTime = 0.25; // Clamp lag spikes

        bool shouldRecord = benchmark.EvaluateFrame(frameTime);

        // --- Get Exact Physical Framebuffer Pixels ---
        int curPixelW = winW, curPixelH = winH;
        BruhEngine::GetDrawableSize(&curPixelW, &curPixelH);
        // ---------------------------------------------

        if (shouldRecord) {
            benchmark.RecordFrame(
                (float)(frameTime * 1000.0),
                overdrawFactor,
                curPixelW,
                curPixelH,
                IsBlendEnabled(),
                IsDepthEnabled(),
                IsVSyncEnabled()
            );
        }
        telemetry.Update(frameTime);
        telemetry.CalculateGpix((float)curPixelW, (float)curPixelH, overdrawFactor);

        // Skip ImGui CPU overhead when HUD is hidden
        BruhEngine::SetImGuiEnabled(telemetry.IsVisible());

        BeginBatch();
        for (const auto& p : particles) {
            DrawQuad(p.x, p.y, spriteWidth, spriteHeight, p.color);
        }

        // --- Render Modular Telemetry HUD ---
        if (telemetry.IsVisible()) {
            telemetry.DrawHUD(
                particleCount,
                GetDrawCalls(),
                overdrawFactor,
                IsVSyncEnabled(),
                [](bool vsync) { SetVSync(vsync); },
                IsDepthEnabled(),
                [](bool depth) { SetDepthEnabled(depth); }
            );
        }

        EndBatch();

        // Display performance stats in title bar
        int drawCalls = GetDrawCalls();
        std::string vsyncStr = IsVSyncEnabled() ? "On" : "Off";
        std::string title = "BruhEngine | Overdraw: ~" + std::to_string((int)overdrawFactor) +
                            "x | Draw Calls: " + std::to_string(drawCalls) +
                            " | VSync: " + vsyncStr;
        SetWindowTitle(title);
    };

    // Run engine loop with custom update and render callbacks
    Run(updateLogic, renderLogic, warmupTimer);

    benchmark.FinalizeAndPrint();

    telemetry.Shutdown();
    Shutdown();
    return 0;
}
