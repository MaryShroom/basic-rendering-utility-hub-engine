#include "HardwareSampler.hpp"
#include <fstream>
#include <string>

HardwareSampler::HardwareSampler() = default;

HardwareMetrics HardwareSampler::Sample() {
    HardwareMetrics metrics;

    // Read CPU Load from /proc/stat
    {
        std::ifstream statFile("/proc/stat");
        if (statFile.is_open()) {
            std::string cpu;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
            statFile >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
            unsigned long long active = user + nice + system + irq + softirq + steal;
            unsigned long long total = active + idle + iowait;

            if (m_InitializedCpuStats) {
                unsigned long long deltaActive = active - m_LastCpuActive;
                unsigned long long deltaTotal = total - m_LastCpuTotal;
                if (deltaTotal > 0) {
                    metrics.cpuLoad = (double)deltaActive / (double)deltaTotal * 100.0;
                }
            }
            m_LastCpuActive = active;
            m_LastCpuTotal = total;
            m_InitializedCpuStats = true;
        }
    }

    // Read GPU Load from /sys/class/drm/card*/device/gpu_busy_percent
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
                if (busy >= 0 && busy <= 100) {
                    metrics.gpuLoad = (double)busy;
                    break;
                }
            } catch (...) {}
        }
    }

    // Read GPU Temp from /sys/class/hwmon/hwmon* (expanded range up to 64)
    for (int i = 0; i < 64; ++i) {
        std::string hwmonDir = "/sys/class/hwmon/hwmon" + std::to_string(i) + "/";
        std::ifstream nameFile(hwmonDir + "name");
        if (nameFile.is_open()) {
            std::string driverName;
            nameFile >> driverName;
            // Added "radeon" for legacy driver compatibility
            if (driverName == "amdgpu" || driverName == "radeon" || driverName == "nvidia" || driverName == "nouveau") {
                std::ifstream tempFile(hwmonDir + "temp1_input");
                if (tempFile.is_open()) {
                    long long tempMilliC = 0; // Properly initialized to prevent uninitialized reads
                    if (tempFile >> tempMilliC) {
                        double tempC = tempMilliC / 1000.0;
                        if (tempC > 0 && tempC < 150) {
                            metrics.gpuTemp = tempC;
                            break;
                        }
                    }
                }
            }
        }
    }

    return metrics;
}
