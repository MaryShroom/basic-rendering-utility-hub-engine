#include "BenchmarkRunner.hpp"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

void BenchmarkRunner::Configure(bool enabled, const std::string& testName, float overdrawFactor) {
    m_Enabled = enabled;
    m_TestName = testName;
    m_OverdrawFactor = overdrawFactor;
    m_FrameTimes.clear();

    m_FrameTimes.reserve(60000);
}

bool BenchmarkRunner::EvaluateFrame(double frameTime, bool& isRunning) {
    if (!m_Enabled) return true;

    // 100 ms cut-off safety net
    if (frameTime > 0.100) {
        std::cerr << "\n[BenchmarkRunner] CRITICAL: Frame time exceeded 100ms ("
        << std::fixed << std::setprecision(2) << (frameTime * 1000.0)
        << " ms). Automatic cut-off triggered.\n";
        isRunning = false;
        return false;
    }
    return true;
}

void BenchmarkRunner::RecordFrameTime(float ms) {
    if (m_Enabled) {
        m_FrameTimes.push_back(ms);
    }
}

void BenchmarkRunner::FinalizeAndPrint() {
    if (!m_Enabled || m_FrameTimes.empty()) return;

    float avgMs = std::accumulate(m_FrameTimes.begin(), m_FrameTimes.end(), 0.0f) / m_FrameTimes.size();
    float worstMs = *std::max_element(m_FrameTimes.begin(), m_FrameTimes.end());

    std::cout << "\n========================================\n";
    std::cout << "          BRUH ENGINE BENCHMARK         \n";
    std::cout << "========================================\n";
    std::cout << "Test Name:   " << m_TestName << "\n";
    std::cout << "Overdraw:    " << m_OverdrawFactor << "x\n";
    std::cout << "Avg Frame:   " << std::fixed << std::setprecision(2) << avgMs << " ms\n";
    std::cout << "Worst Frame: " << worstMs << " ms\n";
    std::cout << "Total Frames Logged: " << m_FrameTimes.size() << "\n";
    std::cout << "========================================\n";
}
