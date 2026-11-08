#include "engine/bruh.hpp"

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
    if (!Init("BruhEngine - Stress Test", 700, 700)) {
        return -1;
    }

    // Register event listener to toggle VSync on pressing 'V'
    SetEventCallback([](const SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_v) {
                SetVSync(!IsVSyncEnabled());
            }
        }
    });

    // Initialize 8,000 moving particles
    const int particleCount = 60000;
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

    float spriteWidth = 0.1f;
    float spriteHeight = 0.1f;

    float overdrawFactor = (particleCount * (0.1f * 0.1f)) / 4.0f;
    BruhEngine::SetBenchmarkInfo("60k Sprites (0.1x)", "~" + std::to_string((int)overdrawFactor) + "x");

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
        // --- Overdraw Calculation ---
        float screenArea = 4.0f; // NDC space width (2) * height (2)
        float totalSpriteArea = (float)particleCount * (spriteWidth * spriteHeight);
        float overdrawFactor = totalSpriteArea / screenArea;
        // ----------------------------

        BeginBatch();
        for (const auto& p : particles) {
            DrawQuad(p.x, p.y, 0.1f, 0.1f, p.color);
        }

        // --- ImGui Performance HUD Overlay ---
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.75f);

        if (ImGui::Begin("Engine Metrics", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
            ImGuiIO& io = ImGui::GetIO();

            ImGui::Text("Performance");
            ImGui::Text("FPS: %.1f (%.3f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
            ImGui::Separator();

            ImGui::Text("Active Sprites: %d", particleCount);
            ImGui::Text("Draw Calls: %d", GetDrawCalls());
            ImGui::Text("Est. Overdraw: ~%.0fx", overdrawFactor);
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

    Shutdown();
    return 0;
}
