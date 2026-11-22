#pragma once
#include <functional>
#include <string>
#include <SDL.h>

namespace BruhApplication {
    bool InitWindow(const char* title, int width, int height, Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    void Run(
        std::function<void(float fixedDeltaTime)> updateFunc,
        std::function<void()> renderFunc,
        std::function<void()> presentFunc,
        double warmupGraceSeconds = 2.0
    );
    void ShutdownWindow();

    SDL_Window* GetNativeWindow();
    int GetWindowWidth();
    int GetWindowHeight();
    void SetWindowTitle(const std::string& title);
    void GetWindowSize(int* w, int* h);
    void GetDrawableSize(int* w, int* h);

    void SetEventCallback(std::function<void(const SDL_Event&)> callback);
    void SetResizeCallback(std::function<void(int drawableW, int drawableH)> callback);
    void SetImGuiEnabled(bool enabled);
    bool IsImGuiEnabled();
}
