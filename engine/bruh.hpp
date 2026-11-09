#pragma once
#include <functional>
#include <string>
#include <SDL.h>

namespace BruhEngine {
    bool Init(const char* title, int width, int height);
    void Run(std::function<void(float fixedDeltaTime)> updateFunc, std::function<void()> renderFunc);
    void SetWindowTitle(const std::string& title);
    void GetWindowSize(int* w, int* h);
    void Shutdown();

    // VSync controls
    bool SetVSync(bool enabled);
    bool IsVSyncEnabled();

    // Draw Calls
    int GetDrawCalls();
    void IncrementDrawCalls();

    // Event Handling Hook
    void SetEventCallback(std::function<void(const SDL_Event&)> callback);

    // Batching hook
    void BeginBatch();
    void DrawQuad(float x, float y, float width, float height, const float color[3]);
    void EndBatch();

    // Benchmark
    void SetBenchmarkInfo(const std::string& testName, float overdrawFactor);
    void SetBenchmarkMetadata(const std::string& pc, const std::string& commit, const std::string& notes);

    void SetBlendEnabled(bool enabled);
    void SetDepthEnabled(bool enabled);
    bool IsDepthEnabled();
}
