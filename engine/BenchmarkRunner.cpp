#include "BenchmarkRunner.hpp"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

void BenchmarkRunner::Configure(bool enabled, std::string testName, float expectedOverdraw,
                                double warmupSeconds = 2.0, double runSeconds = 10.0) {
    m_Enabled = enabled;
    m_TestName = testName;
    m_FrameTimes.clear();
    m_OverdrawSamples.clear();
    m_PixelCounts.clear();
    m_GpixSamples.clear();
    m_GPULoadSamples.clear();
    m_CPULoadSamples.clear();
    m_GPUTempSamples.clear();

    m_FrameTimes.reserve(60000);
    m_OverdrawSamples.reserve(60000);
    m_PixelCounts.reserve(60000);
    m_GpixSamples.reserve(60000);
    m_GPULoadSamples.reserve(60000);
    m_CPULoadSamples.reserve(60000);
    m_GPUTempSamples.reserve(60000);

    m_WarmupDuration = warmupSeconds;
    m_RunLength = runSeconds;
    m_CurrentWarmupTime = 0.0;
    m_CurrentRunTime = 0.0;
    m_WarmupComplete = false;
    m_BenchmarkComplete = false;
}

bool BenchmarkRunner::EvaluateFrame(double frameTime, bool& isRunning) {
    if (!m_Enabled) return true;

    if (!m_WarmupComplete) {
        m_CurrentWarmupTime += frameTime;
        if (m_CurrentWarmupTime >= m_WarmupDuration) {
            m_WarmupComplete = true;
            std::cout << "[Benchmark] Warm-up complete. Starting active collection window...\n";
        }
        return false; // Do not record during warm-up
    }

    if (m_BenchmarkComplete) {
        return false;
    }

    m_CurrentRunTime += frameTime;
    if (m_CurrentRunTime >= m_RunLength) {
        m_BenchmarkComplete = true;
        FinalizeAndPrint();
        std::cout << "[Benchmark] Collection complete. Window remains active.\n";
        return false; // Stop recording, but keep window running
    }

    return true; // Frame approved for recording
}

void BenchmarkRunner::RecordFrame(float ms, double overdrawFactor, int windowWidth, int windowHeight,
                                  bool blendActive, bool depthActive, bool vsyncActive,
                                  const std::string& gpuLoadStr, const std::string& cpuLoadStr, const std::string& tempStr) {
    if (!m_Enabled) return;

    m_FrameTimes.push_back(ms);
    m_OverdrawSamples.push_back(overdrawFactor);

    double framePixels = (double)windowWidth * (double)windowHeight;
    m_PixelCounts.push_back(framePixels);

    // Per-frame instantaneous throughput sample (ms converted to seconds)
    double frameSeconds = (double)ms / 1000.0;
    double frameFps = (frameSeconds > 0.0) ? (1.0 / frameSeconds) : 0.0;
    double frameGpix = (overdrawFactor * framePixels * frameFps) / 1'000'000'000.0;
    m_GpixSamples.push_back(frameGpix);

    m_LastWinW = windowWidth;
    m_LastWinH = windowHeight;
    m_LastBlend = blendActive;
    m_LastDepth = depthActive;
    m_LastVSync = vsyncActive;

    try {
        if (gpuLoadStr != "N/A" && !gpuLoadStr.empty()) {
            m_GPULoadSamples.push_back(std::stod(gpuLoadStr));
        }
    } catch (...) {}

    try {
        if (cpuLoadStr != "N/A" && !cpuLoadStr.empty()) {
            m_CPULoadSamples.push_back(std::stod(cpuLoadStr));
        }
    } catch (...) {}

    try {
        if (tempStr != "N/A" && !tempStr.empty()) {
            m_GPUTempSamples.push_back(std::stod(tempStr));
        }
    } catch (...) {}
}

void BenchmarkRunner::FinalizeAndPrint() {
    if (!m_Enabled || m_FrameTimes.empty()) return;

    // Sum of all frame times in milliseconds, converted to total elapsed seconds
    double totalMs = std::accumulate(m_FrameTimes.begin(), m_FrameTimes.end(), 0.0);
    double totalElapsedSeconds = totalMs / 1000.0;

    float avgMs = (float)(totalMs / m_FrameTimes.size());
    float worstMs = *std::max_element(m_FrameTimes.begin(), m_FrameTimes.end());
    float avgFPS = (totalElapsedSeconds > 0.0) ? (float)(m_FrameTimes.size() / totalElapsedSeconds) : 0.0f;

    double avgOverdraw = std::accumulate(m_OverdrawSamples.begin(), m_OverdrawSamples.end(), 0.0) / m_OverdrawSamples.size();

    // 1. Derived Throughput (Arithmetic mean of per-frame instantaneous Gpix/s)
    double derivedThroughput = std::accumulate(m_GpixSamples.begin(), m_GpixSamples.end(), 0.0) / m_GpixSamples.size();

    // 2. Run-Average Gpix/s: Total gigapixels submitted across all frames ÷ Total Elapsed Seconds
    double totalGpixelsSubmitted = 0.0;
    for (size_t i = 0; i < m_FrameTimes.size(); ++i) {
        totalGpixelsSubmitted += (m_OverdrawSamples[i] * m_PixelCounts[i]) / 1'000'000'000.0;
    }
    double runAverageGpixPerSec = (totalElapsedSeconds > 0.0) ? (totalGpixelsSubmitted / totalElapsedSeconds) : 0.0;

    double avgGPULoad = m_GPULoadSamples.empty() ? 0.0 : std::accumulate(m_GPULoadSamples.begin(), m_GPULoadSamples.end(), 0.0) / m_GPULoadSamples.size();
    double avgCPULoad = m_CPULoadSamples.empty() ? 0.0 : std::accumulate(m_CPULoadSamples.begin(), m_CPULoadSamples.end(), 0.0) / m_CPULoadSamples.size();
    double avgGPUTemp = m_GPUTempSamples.empty() ? 0.0 : std::accumulate(m_GPUTempSamples.begin(), m_GPUTempSamples.end(), 0.0) / m_GPUTempSamples.size();

    std::string windowCol = std::to_string(m_LastWinW) + "x" + std::to_string(m_LastWinH);
    std::string blendCol  = m_LastBlend ? "on" : "off";
    std::string depthCol  = m_LastDepth ? "on" : "off";
    std::string vsyncCol  = m_LastVSync ? "on" : "off";
    std::string gpuLoadCol = m_GPULoadSamples.empty() ? "N/A" : std::to_string((int)avgGPULoad) + "%";
    std::string cpuLoadCol = m_CPULoadSamples.empty() ? "N/A" : std::to_string((int)avgCPULoad) + "%";
    std::string tempCol    = m_GPUTempSamples.empty() ? "N/A" : std::to_string((int)avgGPUTemp) + " C";

    // Format duration values
    std::string warmupCol  = std::to_string((int)m_WarmupDuration) + " s";
    std::string runLenCol  = std::to_string((int)m_RunLength) + " s";

    std::cout << "\n========================================================================================\n";
    std::cout << "                                BRUHENGINE BENCHMARK RESULT                             \n";
    std::cout << "========================================================================================\n";
    std::cout << " Test Name:             " << m_TestName << "\n";
    std::cout << " Window Size:           " << windowCol << "\n";
    std::cout << " Warm-up Duration:      " << warmupCol << "\n";
    std::cout << " Active Run Duration:   " << runLenCol << "\n";
    std::cout << " Blend:                 " << blendCol << "\n";
    std::cout << " Depth Test:            " << depthCol << "\n";
    std::cout << " VSync:                 " << vsyncCol << "\n";
    std::cout << " Avg Overdraw:          ~" << std::fixed << std::setprecision(1) << avgOverdraw << "x\n";
    std::cout << " Avg Frame Time:        " << std::setprecision(2) << avgMs << " ms\n";
    std::cout << " Worst Frame Time:      " << worstMs << " ms\n";
    std::cout << " Average FPS:           " << std::setprecision(1) << avgFPS << "\n";
    std::cout << " Derived Throughput:    " << std::setprecision(3) << derivedThroughput << " Gpix/s\n";
    std::cout << " Run-Average Gpix/s:    " << std::setprecision(3) << runAverageGpixPerSec << " Gpix/s\n";
    std::cout << " Avg GPU Load:          " << gpuLoadCol << "\n";
    std::cout << " Avg CPU Load:          " << cpuLoadCol << "\n";
    std::cout << " Avg GPU Temp:          " << tempCol << "\n";
    std::cout << " Total Frames Logged:   " << m_FrameTimes.size() << "\n";
    std::cout << "========================================================================================\n\n";
}
