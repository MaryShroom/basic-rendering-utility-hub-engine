#pragma once
#include <string>
#include <vector>

class BenchmarkRunner {
public:
    void Configure(bool enabled, const std::string& testName, float overdrawFactor);

    // Returns false if the 100ms cut-off was triggered (signals loop termination)
    bool EvaluateFrame(double frameTime, bool& isRunning);

    void RecordFrameTime(float ms);
    void FinalizeAndPrint();

    bool IsEnabled() const { return m_Enabled; }

private:
    bool m_Enabled = false;
    std::string m_TestName = "Default";
    float m_OverdrawFactor = 1.0f;
    std::vector<float> m_FrameTimes;
};
