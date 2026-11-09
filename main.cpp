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
        SetVSync(false); // Toggle V-Sync
    } else {
        BruhEngine::SetBlendEnabled(true);
    }

    // PC, Commit, Notes
    BruhEngine::SetBenchmarkMetadata("B", "e615b36", "10k particles. Particle size of 8.0 by 8.0 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build.");

    // Register event listener to toggle VSync on pressing 'V'
    SetEventCallback([](const SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_v) {
                SetVSync(!IsVSyncEnabled());
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
        telemetry.CalculateGpix((float)winW, (float)winH, overdrawFactor);

        BeginBatch();
        for (const auto& p : particles) {
            DrawQuad(p.x, p.y, spriteWidth, spriteHeight, p.color);
        }

        // --- ImGui Performance HUD Overlay ---
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.75f);

        if (ImGui::Begin("Engine Metrics", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
            float fps = telemetry.GetFPS();
            float frameTimeMs = telemetry.GetFrameTimeMs();

            ImGui::Text("Performance");
            ImGui::Text("FPS: %.1f (%.3f ms/frame)", fps, frameTimeMs);
            ImGui::Text("Uptime: %.1f s", telemetry.GetUptimeSeconds());
            ImGui::Separator();

            // Hardware Telemetry
            ImGui::Text("GPU Load: %s", telemetry.GetGPULoad().c_str());
            ImGui::Text("GPU Temp: %s", telemetry.GetGPUTemp().c_str());
            ImGui::Separator();

            ImGui::Text("Active Sprites: %d", particleCount);
            ImGui::Text("Draw Calls: %d", GetDrawCalls());
            ImGui::Text("Est. Overdraw: ~%.0fx", overdrawFactor);

            // Display in Gigapixels per second for readability (e.g., Gpix/s)
            ImGui::Text("Throughput: %.3f Gpix/s", telemetry.GetGpixPerSec());

            ImGui::Separator();

            bool vsync = IsVSyncEnabled();
            if (ImGui::Checkbox("VSync Enabled", &vsync)) {
                SetVSync(vsync);
            }

            ImGui::End();
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
    Run(updateLogic, renderLogic);

    benchmark.FinalizeAndPrint();
    telemetry.Shutdown();
    Shutdown();
    return 0;
}
