#include "core/Application.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"

#include <iostream>
#include <chrono>
#include <iomanip>

namespace BruhApplication {
    static SDL_Window* s_Window = nullptr;
    static bool s_IsRunning = false;
    static int s_WinW = 700;
    static int s_WinH = 700;
    static bool s_ImGuiEnabled = true;

    static std::function<void(const SDL_Event&)> s_EventCallback = nullptr;
    static std::function<void(int, int)> s_ResizeCallback = nullptr;
    static std::chrono::high_resolution_clock::time_point s_StartTime;

    static void ProcessEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (s_ImGuiEnabled) {
                ImGui_ImplSDL2_ProcessEvent(&e);
            }

            if (e.type == SDL_QUIT) {
                s_IsRunning = false;
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    s_WinW = e.window.data1;
                    s_WinH = e.window.data2;

                    int drawW = s_WinW, drawH = s_WinH;
                    SDL_GL_GetDrawableSize(s_Window, &drawW, &drawH);
                    if (s_ResizeCallback) {
                        s_ResizeCallback(drawW, drawH);
                    }
                }
            }

            if (s_EventCallback) {
                s_EventCallback(e);
            }
        }
    }

    bool InitWindow(const char* title, int width, int height, Uint32 windowFlags) {
        s_WinW = width;
        s_WinH = height;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
            return false;
        }

        s_Window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            windowFlags
        );

        if (!s_Window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }

        s_StartTime = std::chrono::high_resolution_clock::now();
        s_IsRunning = true;
        return true;
    }

    void Run(
        std::function<void(float fixedDeltaTime)> updateFunc,
        std::function<void()> renderFunc,
        std::function<void()> presentFunc,
        double warmupGraceSeconds
    ) {
        const double targetFPS = 60.0;
        const double fixedDt = 1.0 / targetFPS;

        Uint64 lastCounter = SDL_GetPerformanceCounter();
        double accumulator = 0.0;

        while (s_IsRunning) {
            ProcessEvents();

            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 counterFrequency = SDL_GetPerformanceFrequency();
            double frameTime = (double)(currentCounter - lastCounter) / (double)counterFrequency;
            lastCounter = currentCounter;

            double engineUptime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - s_StartTime).count();
            if (frameTime > 0.100 && engineUptime > warmupGraceSeconds) {
                std::cerr << "\n[BruhEngine] CRITICAL: Frame time exceeded 100ms ("
                          << std::fixed << std::setprecision(2) << (frameTime * 1000.0)
                          << " ms). Automatic cut-off triggered.\n";
                s_IsRunning = false;
                break;
            }

            if (frameTime > 0.25) frameTime = 0.25;
            accumulator += frameTime;

            while (accumulator >= fixedDt) {
                if (updateFunc) updateFunc((float)fixedDt);
                accumulator -= fixedDt;
            }

            if (renderFunc) renderFunc();
            if (presentFunc) presentFunc();
        }
    }

    void ShutdownWindow() {
        if (s_Window) {
            SDL_DestroyWindow(s_Window);
            s_Window = nullptr;
        }
        SDL_Quit();
        std::cout << "BruhEngine Shut Down Cleanly.\n";
    }

    SDL_Window* GetNativeWindow() { return s_Window; }
    int GetWindowWidth() { return s_WinW; }
    int GetWindowHeight() { return s_WinH; }

    void SetWindowTitle(const std::string& title) {
        if (s_Window) SDL_SetWindowTitle(s_Window, title.c_str());
    }

    void GetWindowSize(int* w, int* h) {
        if (s_Window) SDL_GetWindowSize(s_Window, w, h);
    }

    void GetDrawableSize(int* w, int* h) {
        if (s_Window) SDL_GL_GetDrawableSize(s_Window, w, h);
    }

    void SetEventCallback(std::function<void(const SDL_Event&)> callback) {
        s_EventCallback = callback;
    }

    void SetResizeCallback(std::function<void(int drawableW, int drawableH)> callback) {
        s_ResizeCallback = callback;
    }

    void SetImGuiEnabled(bool enabled) { s_ImGuiEnabled = enabled; }
    bool IsImGuiEnabled() { return s_ImGuiEnabled; }
}
