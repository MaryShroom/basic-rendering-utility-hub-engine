#include "TelemetryMonitor.hpp"
#include "imgui.h"

void TelemetryMonitor::Init() {
    m_StartTime = std::chrono::high_resolution_clock::now();

    m_Running = true;
    // Launch background thread for sysfs polling
    m_WorkerThread = std::thread(&TelemetryMonitor::PollLoop, this);
}

void TelemetryMonitor::Shutdown() {
    m_Running = false;
    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join(); // Safely wait for background thread to exit
    }
}

void TelemetryMonitor::PollLoop() {
    while (m_Running) {
        HardwareMetrics metrics = m_Sampler.Sample();

        // Fetch hardware stats in the background without affecting render performance
        std::string gpuLoadStr = (metrics.gpuLoad >= 0.0) ? std::to_string((int)metrics.gpuLoad) + "%" : "N/A";
        std::string gpuTempStr = (metrics.gpuTemp > 0.0) ? std::to_string((int)metrics.gpuTemp) + " C" : "N/A";

        // Safely update cached values
        {
            std::lock_guard<std::mutex> lock(m_DataMutex);
            m_CachedGPULoad = gpuLoadStr;
            m_CachedGPUTemp = gpuTempStr;
        }

        // Sleep for 1 second before polling again
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void TelemetryMonitor::Update(double frameTime) {
    m_FrameTimeMs = (float)(frameTime * 1000.0);
    m_FPS = (frameTime > 0.0) ? (1.0f / (float)frameTime) : 0.0f;
}

std::string TelemetryMonitor::GetGPULoad() const {
    std::lock_guard<std::mutex> lock(m_DataMutex);
    return m_CachedGPULoad;
}

std::string TelemetryMonitor::GetGPUTemp() const {
    std::lock_guard<std::mutex> lock(m_DataMutex);
    return m_CachedGPUTemp;
}

double TelemetryMonitor::GetUptimeSeconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - m_StartTime;
    return elapsed.count();
}

double TelemetryMonitor::GetGpixPerSec() const {
    std::lock_guard<std::mutex> lock(m_DataMutex);
    return m_GpixPerSec;
}

void TelemetryMonitor::CalculateGpix(float screenWidth, float screenHeight, double overdrawFactor) {
    float frameSeconds = m_FrameTimeMs / 1000.0f;
    m_AccumulatedTime += frameSeconds;
    m_FrameCount++;

    if (m_AccumulatedTime >= 0.5f) {
        float avgFps = (m_AccumulatedTime > 0.0f) ? ((float)m_FrameCount / m_AccumulatedTime) : 0.0f;
        double windowPixels = (double)screenWidth * (double)screenHeight;
        {
            std::lock_guard<std::mutex> lock(m_DataMutex);
            m_GpixPerSec = (windowPixels * overdrawFactor * (double)avgFps) / 1'000'000'000.0;
        }
        m_AccumulatedTime = 0.0f;
        m_FrameCount = 0;
    }
}

void TelemetryMonitor::SetVisible(bool visible) {
    m_ShowHUD = visible;
}

bool TelemetryMonitor::IsVisible() const {
    return m_ShowHUD;
}

void TelemetryMonitor::ToggleVisible() {
    m_ShowHUD = !m_ShowHUD;
}

void TelemetryMonitor::DrawHUD(int particleCount, int drawCalls, float overdrawFactor, bool vsyncEnabled, std::function<void(bool)> onVSyncToggle, bool depthEnabled, std::function<void(bool)> onDepthToggle) {
    if (!m_ShowHUD) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.75f);

    if (ImGui::Begin("Engine Metrics", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        ImGui::Text("Performance");
        ImGui::Text("FPS: %.1f (%.3f ms/frame)", m_FPS, m_FrameTimeMs);
        ImGui::Text("Uptime: %.1f s", GetUptimeSeconds());
        ImGui::Separator();

        // Hardware Telemetry
        ImGui::Text("GPU Load: %s", GetGPULoad().c_str());
        ImGui::Text("GPU Temp: %s", GetGPUTemp().c_str());
        ImGui::Separator();

        ImGui::Text("Active Sprites: %d", particleCount);
        ImGui::Text("Draw Calls: %d", drawCalls);
        ImGui::Text("Est. Overdraw: ~%.0fx", overdrawFactor);

        // Throughput metrics
        ImGui::Text("Throughput: %.3f Gpix/s", m_GpixPerSec);

        ImGui::Separator();

        bool vsync = vsyncEnabled;
        if (ImGui::Checkbox("VSync Enabled", &vsync)) {
            if (onVSyncToggle) {
                onVSyncToggle(vsync);
            }
        }

        bool depth = depthEnabled;
        if (ImGui::Checkbox("Depth Test Enabled", &depth)) {
            if (onDepthToggle) {
                onDepthToggle(depth);
            }
        }

        ImGui::End();
    }
}
