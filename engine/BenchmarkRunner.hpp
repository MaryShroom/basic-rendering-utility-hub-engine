#pragma once
#include <string>
#include <vector>
#include <fstream>

#include "HardwareSampler.hpp"

class BenchmarkRunner {
public:
    void Configure(bool enabled, std::string testName, float /*expectedOverdraw*/,
                   double warmupSeconds, double runSeconds);

    // Returns false if the 100ms cut-off was triggered (signals loop termination)
    bool EvaluateFrame(double frameTime, bool& /*isRunning*/);

    void RecordFrame(float ms, double overdrawFactor, int windowWidth, int windowHeight,
                     bool blendActive, bool depthActive, bool vsyncActive);

    void FinalizeAndPrint();

    bool IsEnabled() const { return m_Enabled; }

    double GetWarmupDuration() const { return m_WarmupDuration; }

private:
    void SampleHardware();

    bool m_Enabled = false;
    std::string m_TestName = "Default";
    std::vector<float> m_FrameTimes;
    std::vector<double> m_OverdrawSamples;
    std::vector<double> m_PixelCounts;
    std::vector<double> m_GpixSamples;

    // Hardware and state samples for averaging
    std::vector<double> m_GPULoadSamples;
    std::vector<double> m_CPULoadSamples;
    std::vector<double> m_GPUTempSamples;

    int m_LastWinW = 700;
    int m_LastWinH = 700;
    bool m_LastBlend = true;
    bool m_LastDepth = false;
    bool m_LastVSync = false;

    double m_WarmupDuration = 2.0;
    double m_RunLength = 10.0;
    double m_CurrentWarmupTime = 0.0;
    double m_CurrentRunTime = 0.0;
    double m_HardwareSampleTimer = 0.0;

    bool m_WarmupComplete = false;
    bool m_BenchmarkComplete = false;

    // CPU delta state tracking
    HardwareSampler m_Sampler;
};
