#pragma once
#include <string>

struct HardwareMetrics {
    double cpuLoad = -1.0; // Percentage [0, 100]
    double gpuLoad = -1.0; // Percentage [0, 100]
    double gpuTemp = -1.0; // Temperature in Celsius
};

class HardwareSampler {
public:
    HardwareSampler();
    HardwareMetrics Sample();

private:
    unsigned long long m_LastCpuActive = 0;
    unsigned long long m_LastCpuTotal = 0;
    bool m_InitializedCpuStats = false;
};
