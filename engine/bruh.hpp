#pragma once
#include <functional>
#include <string>

namespace BruhEngine {
    bool Init(const char* title, int width, int height);
    void Run(std::function<void(float fixedDeltaTime)> updateFunc, std::function<void()> renderFunc);
    void SetWindowTitle(const std::string& title);
    void GetWindowSize(int* w, int* h);
    void Shutdown();
}
