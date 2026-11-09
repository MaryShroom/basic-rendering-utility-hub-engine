#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

class TelemetryMonitor {
public:
    void Init();
    void Shutdown(); // Clean up thread on exit
    void Update(double frameTime);

    // Thread-safe getters
    float GetFPS() const { return m_FPS; }
    float GetFrameTimeMs() const { return m_FrameTimeMs; }
    std::string GetGPULoad() const;
    std::string GetGPUTemp() const;
    std::string GetCPULoad() const;
    float GetGpixPerSec() const { return m_GpixPerSec; }

    double GetUptimeSeconds() const;

    void CalculateGpix(float screenWidth, float screenHeight, float overdrawFactor);

    // --- ImGui HUD Management ---
    void SetVisible(bool visible) { m_ShowHUD = visible; }
    void ToggleVisible() { m_ShowHUD = !m_ShowHUD; }
    bool IsVisible() const { return m_ShowHUD; }

    void DrawHUD(int particleCount, int drawCalls, float overdrawFactor, bool vsyncEnabled, std::function<void(bool)> onVSyncToggle);

private:
    std::chrono::high_resolution_clock::time_point m_StartTime;

    void PollLoop(); // Background thread function

    float m_FrameTimeMs = 0.0f;
    float m_FPS = 0.0f;
    float m_GpixPerSec = 0.0f;

    // Thread synchronization & caching
    std::thread m_WorkerThread;
    std::atomic<bool> m_Running{false};
    mutable std::mutex m_DataMutex;

    std::string m_CachedGPULoad = "N/A";
    std::string m_CachedGPUTemp = "N/A";

    // Internal file-reading helpers
    std::string ReadGPULoad() const;
    std::string ReadGPUTemp() const;

    bool m_ShowHUD = true;
};
