#pragma once
#include <SDL.h>

namespace RenderContext {
    void ConfigureAttributes();
    bool Init(SDL_Window* window);
    void Shutdown();

    void SetViewport(int width, int height);
    void Clear(float r = 0.1f, float g = 0.12f, float b = 0.16f, float a = 1.0f);
    void SwapBuffers(SDL_Window* window);

    void BeginImGuiFrame();
    void EndImGuiFrame();

    bool SetVSync(bool enabled);
    bool IsVSyncEnabled();

    void SetBlendEnabled(bool enabled);
    bool IsBlendEnabled();

    void SetDepthEnabled(bool enabled);
    bool IsDepthEnabled();
}
