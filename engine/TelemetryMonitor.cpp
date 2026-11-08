#include "TelemetryMonitor.hpp"

void TelemetryMonitor::Init() {
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
    for (int i = 0; i <= 5; ++i) {
        std::string path = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/temp1_input";
        std::ifstream file(path);
        if (file.is_open()) {
            std::string valStr;
            std::getline(file, valStr);
            try {
                float temp = std::stoi(valStr) / 1000.0f;
                return std::to_string((int)temp) + "°C";
            } catch (...) {}
        }
    }
    return "N/A";
}

void TelemetryMonitor::CalculateGpix(float screenWidth, float screenHeight, float overdrawFactor) {
    float windowPixels = screenWidth * screenHeight;
    m_GpixPerSec = (windowPixels * overdrawFactor * m_FPS) / 1'000'000.0f;
}
