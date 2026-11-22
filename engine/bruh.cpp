#include "bruh.hpp"
#include "core/Application.hpp"
#include "renderer/gl/RenderContext.hpp"
#include "renderer/gl/Shader.hpp"
#include "renderer/gl/Texture.hpp"
#include "renderer2d/OrthographicCamera.hpp"
#include "renderer2d/SpriteBatcher.hpp"

#include <array>

namespace BruhEngine {
    static int s_DrawCalls = 0;
    static Shader* s_Shader = nullptr;
    static Texture* s_Texture = nullptr;
    static SpriteBatcher* s_Batcher = nullptr;
    static OrthographicCamera s_Camera;

    static const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 u_ViewProjection;\n"
        "out vec3 fragColor;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "   gl_Position = u_ViewProjection * vec4(aPos, 0.0, 1.0);\n"
        "   fragColor = aColor;\n"
        "   TexCoord = aTexCoord;\n"
        "}\n";

    static const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 fragColor;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D u_Texture;\n"
        "void main() {\n"
        "   FragColor = texture(u_Texture, TexCoord) * vec4(fragColor, 1.0);\n"
        "}\n";

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

        s_Shader = new Shader(vertexShaderSource, fragmentShaderSource);
        s_Texture = new Texture(64, 64);
        s_Batcher = new SpriteBatcher();
        return true;
    }

    void Run(std::function<void(float fixedDeltaTime)> updateFunc, std::function<void()> renderFunc, double warmupGraceSeconds) {
        BruhApplication::Run(
            updateFunc,
            [&renderFunc]() {
                s_DrawCalls = 0;
                if (renderFunc) renderFunc();
            },
            []() {
                RenderContext::SwapBuffers(BruhApplication::GetNativeWindow());
            },
            warmupGraceSeconds
        );
    }

    void Shutdown() {
        delete s_Batcher;
        delete s_Texture;
        delete s_Shader;
        s_Batcher = nullptr;
        s_Texture = nullptr;
        s_Shader = nullptr;

        RenderContext::Shutdown();
        BruhApplication::ShutdownWindow();
    }

    OrthographicCamera& GetCamera() { return s_Camera; }
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

    int GetDrawCalls() {
        int pending = (s_Batcher && s_Batcher->HasPendingDraws()) ? 1 : 0;
        return s_DrawCalls + pending;
    }

    void IncrementDrawCalls() { s_DrawCalls++; }

    void BeginBatch() {
        RenderContext::Clear();
        s_DrawCalls = 0;

        if (BruhApplication::IsImGuiEnabled()) {
            RenderContext::BeginImGuiFrame();
        }

        if (s_Batcher && s_Shader && s_Texture) {
            s_Batcher->BeginBatch(*s_Shader, *s_Texture);
            std::array<float, 16> proj = s_Camera.GetMatrix(
                (float)BruhApplication::GetWindowWidth(),
                (float)BruhApplication::GetWindowHeight()
            );
            s_Shader->Bind();
            s_Shader->SetUniformMat4("u_ViewProjection", proj.data());
        }
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3]) {
        if (s_Batcher) s_Batcher->DrawQuad(x, y, width, height, color);
    }

    void EndBatch() {
        if (s_Batcher) s_Batcher->Flush();
        if (BruhApplication::IsImGuiEnabled()) {
            RenderContext::EndImGuiFrame();
        }
    }
}
