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
        // Fetch hardware stats in the background without affecting render performance
        std::string gpuLoad = ReadGPULoad();
        std::string gpuTemp = ReadGPUTemp();

        // Safely update cached values
        {
            std::lock_guard<std::mutex> lock(m_DataMutex);
            m_CachedGPULoad = gpuLoad;
            m_CachedGPUTemp = gpuTemp;
        }

        // Sleep for 1 second before polling again
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void TelemetryMonitor::Update(double frameTime) {
    m_FrameTimeMs = (float)(frameTime * 1000.0);
    m_FPS = (frameTime > 0.0) ? (1.0f / (float)frameTime) : 0.0f;
    // No blocking file I/O on the main thread anymore!
}

std::string TelemetryMonitor::GetGPULoad() const {
    std::lock_guard<std::mutex> lock(m_DataMutex);
    return m_CachedGPULoad;
}

std::string TelemetryMonitor::GetGPUTemp() const {
    std::lock_guard<std::mutex> lock(m_DataMutex);
    return m_CachedGPUTemp;
}

std::string TelemetryMonitor::ReadGPULoad() const {
    for (int i = 0; i <= 2; ++i) {
        std::string path = "/sys/class/drm/card" + std::to_string(i) + "/device/gpu_busy_percent";
        std::ifstream file(path);
        if (file.is_open()) {
            std::string valStr;
            std::getline(file, valStr);
            valStr.erase(0, valStr.find_first_not_of(" \n\r\t"));
            valStr.erase(valStr.find_last_not_of(" \n\r\t") + 1);
            try {
                int busy = std::stoi(valStr);
                if (busy >= 0 && busy <= 100) return std::to_string(busy) + "%";
            } catch (...) {}
        }
    }
    return "N/A";
}

std::string TelemetryMonitor::ReadGPUTemp() const {
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
    return "N/A";
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
    double windowPixels = (double)screenWidth * (double)screenHeight;
    std::lock_guard<std::mutex> lock(m_DataMutex);
    m_GpixPerSec = (windowPixels * overdrawFactor * (double)m_FPS) / 1'000'000'000.0;
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
