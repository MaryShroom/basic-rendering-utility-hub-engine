#include "engine/bruh.hpp"
#include "engine/TelemetryMonitor.hpp"
#include "engine/BenchmarkRunner.hpp"

#include "imgui.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <SDL.h>

using namespace BruhEngine;

struct Particle {
    float x, y;
    float vx, vy;
    float color[3];
};

int main() {
    int winW = 700, winH = 700;

    if (!Init("BruhEngine - Stress Test", winW, winH)) {
        return -1;
    }

    // Particles count
    const int particleCount = 10000; // Switch to 4000 (10x) or 8000 (20x) as needed

    // Particles size
    float spriteWidth = 8.0f;
    float spriteHeight = 8.0f;

    float overdrawFactor = ((float)particleCount * (spriteWidth * spriteHeight)) / 4.0f;

    // Benchmark and Telemetry Opt-In
    bool enableBenchmarkMode = true;

    TelemetryMonitor telemetry;
    telemetry.Init();

    BenchmarkRunner benchmark;
    if (enableBenchmarkMode) {
        benchmark.Configure(true, std::to_string(particleCount) + " Sprites (8.0x)", overdrawFactor);

        // For my eyes
        BruhEngine::SetBenchmarkInfo(std::to_string(particleCount) + " Sprites (8.0x)", overdrawFactor);

        BruhEngine::SetBlendEnabled(true); // Toggle blend for fill-rate testing
        SetDepthEnabled(false); // Depth Toggle
        SetVSync(false); // Toggle V-Sync
    } else {
        BruhEngine::SetBlendEnabled(true);
    }

    // PC, Commit, Notes
    BruhEngine::SetBenchmarkMetadata("B", "e615b36", "10k particles. Particle size of 8.0 by 8.0 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build.");

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

    bool isRunning = true;

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

    // Custom render logic: what to draw each frame
    auto renderLogic = [&]() {
        float realFps = ImGui::GetIO().Framerate;
        float realFrameTime = (realFps > 0.0f) ? (1.0f / realFps) : 0.0f;

        if (!benchmark.EvaluateFrame(realFrameTime, isRunning)) {
            return;
        }

        benchmark.RecordFrameTime((float)(realFrameTime * 1000.0f));

        telemetry.Update(realFrameTime);

        BruhEngine::GetWindowSize(&winW, &winH);

        // --- Compute Precise Viewport-Clamped Overdraw (Double Precision) ---
        double totalVisibleArea = 0.0;
        for (const auto& p : particles) {
            float left = p.x - (spriteWidth * 0.5f);
            float right = p.x + (spriteWidth * 0.5f);
            float bottom = p.y - (spriteHeight * 0.5f);
            float top = p.y + (spriteHeight * 0.5f);

            // Clamp edges to NDC viewport range [-1.0, 1.0]
            float cLeft = std::max(-1.0f, std::min(1.0f, left));
            float cRight = std::max(-1.0f, std::min(1.0f, right));
            float cBottom = std::max(-1.0f, std::min(1.0f, bottom));
            float cTop = std::max(-1.0f, std::min(1.0f, top));

            float cWidth = cRight - cLeft;
            float cHeight = cTop - cBottom;

            if (cWidth > 0.0f && cHeight > 0.0f) {
                totalVisibleArea += (double)(cWidth * cHeight);
            }
        }
        double overdrawFactor = totalVisibleArea / 4.0;
        // ------------------------------------------------------------------

        telemetry.CalculateGpix((float)winW, (float)winH, overdrawFactor);

        BeginBatch();
        for (const auto& p : particles) {
            DrawQuad(p.x, p.y, spriteWidth, spriteHeight, p.color);
        }

        // --- Render Modular Telemetry HUD ---
        telemetry.DrawHUD(
            particleCount,
            GetDrawCalls(),
            overdrawFactor,
            IsVSyncEnabled(),
            [](bool vsync) { SetVSync(vsync); },
            IsDepthEnabled(),
            [](bool depth) { SetDepthEnabled(depth); }
        );

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
    Run(updateLogic, renderLogic);

    benchmark.FinalizeAndPrint();
    telemetry.Shutdown();
    Shutdown();
    return 0;
}
