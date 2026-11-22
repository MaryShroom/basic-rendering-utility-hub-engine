#include "bruh.hpp"
#include "core/Application.hpp"
#include "renderer/gl/RenderContext.hpp"
#include "renderer2d/Renderer2D.hpp"

namespace BruhEngine {
    bool Init(const char* title, int width, int height) {
        RenderContext::ConfigureAttributes();
        if (!BruhApplication::InitWindow(title, width, height)) {
            return false;
        }
        if (!RenderContext::Init(BruhApplication::GetNativeWindow())) {
            BruhApplication::ShutdownWindow();
            return false;
        }

        BruhApplication::SetResizeCallback([](int drawW, int drawH) {
            RenderContext::SetViewport(drawW, drawH);
        });

        if (!Renderer2D::Init()) {
            RenderContext::Shutdown();
            BruhApplication::ShutdownWindow();
            return false;
        }

        return true;
    }

    void Run(std::function<void(float fixedDeltaTime)> updateFunc, std::function<void()> renderFunc, double warmupGraceSeconds) {
        BruhApplication::Run(
            updateFunc,
            [&renderFunc]() {
                Renderer2D::ResetDrawCalls();
                if (renderFunc) renderFunc();
            },
            []() {
                RenderContext::SwapBuffers(BruhApplication::GetNativeWindow());
            },
            warmupGraceSeconds
        );
    }

    void Shutdown() {
        Renderer2D::Shutdown();
        RenderContext::Shutdown();
        BruhApplication::ShutdownWindow();
    }

    OrthographicCamera& GetCamera() { return Renderer2D::GetCamera(); }
    int GetWindowWidth() { return BruhApplication::GetWindowWidth(); }
    int GetWindowHeight() { return BruhApplication::GetWindowHeight(); }
    void SetWindowTitle(const std::string& title) { BruhApplication::SetWindowTitle(title); }
    void GetWindowSize(int* w, int* h) { BruhApplication::GetWindowSize(w, h); }
    void GetDrawableSize(int* w, int* h) { BruhApplication::GetDrawableSize(w, h); }
    void SetEventCallback(std::function<void(const SDL_Event&)> cb) { BruhApplication::SetEventCallback(cb); }
    void SetImGuiEnabled(bool enabled) { BruhApplication::SetImGuiEnabled(enabled); }

    bool SetVSync(bool enabled) { return RenderContext::SetVSync(enabled); }
    bool IsVSyncEnabled() { return RenderContext::IsVSyncEnabled(); }
    void SetBlendEnabled(bool enabled) { RenderContext::SetBlendEnabled(enabled); }
    bool IsBlendEnabled() { return RenderContext::IsBlendEnabled(); }
    void SetDepthEnabled(bool enabled) { RenderContext::SetDepthEnabled(enabled); }
    bool IsDepthEnabled() { return RenderContext::IsDepthEnabled(); }

    int GetDrawCalls() { return Renderer2D::GetDrawCalls(); }
    void IncrementDrawCalls() { Renderer2D::IncrementDrawCalls(); }

    void BeginBatch() {
        RenderContext::Clear();

        if (BruhApplication::IsImGuiEnabled()) {
            RenderContext::BeginImGuiFrame();
        }

        Renderer2D::BeginBatch(
            (float)BruhApplication::GetWindowWidth(),
            (float)BruhApplication::GetWindowHeight()
        );
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3]) {
        Renderer2D::DrawQuad(x, y, width, height, color);
    }

    void EndBatch() {
        Renderer2D::EndBatch();

        if (BruhApplication::IsImGuiEnabled()) {
            RenderContext::EndImGuiFrame();
        }
    }
}
