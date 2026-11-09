#pragma once
#include <string>
#include <vector>

class BenchmarkRunner {
public:
    void Configure(bool enabled, const std::string& testName, double initialOverdraw = 1.0);

    // Returns false if the 100ms cut-off was triggered (signals loop termination)
    bool EvaluateFrame(double frameTime, bool& isRunning);

    void RecordFrame(float ms, double overdrawFactor, int windowWidth, int windowHeight,
                     bool blendActive, bool depthActive, bool vsyncActive,
                     const std::string& gpuLoadStr, const std::string& cpuLoadStr, const std::string& tempStr);

    void FinalizeAndPrint();

    bool IsEnabled() const { return m_Enabled; }

private:
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
};
