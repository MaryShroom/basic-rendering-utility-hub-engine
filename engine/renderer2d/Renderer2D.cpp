#include "renderer2d/Renderer2D.hpp"
#include "renderer2d/SpriteBatcher.hpp"
#include "renderer/gl/Shader.hpp"
#include "renderer/gl/Texture.hpp"

#include <array>

namespace Renderer2D {
    static int s_DrawCalls = 0;
    static Shader* s_Shader = nullptr;
    static Texture* s_Texture = nullptr;
    static SpriteBatcher* s_Batcher = nullptr;
    static OrthographicCamera s_Camera;

    static const char* s_VertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec3 aColor;
        layout (location = 2) in vec2 aTexCoord;
        layout (location = 3) in vec2 aPivot;
        layout (location = 4) in float aTexID;

        uniform mat4 u_ViewProjection;

        out vec3 fragColor;
        out vec2 TexCoord;

        void main() {
            gl_Position = u_ViewProjection * vec4(aPos, 0.0, 1.0);
            fragColor = aColor;
            TexCoord = aTexCoord;
        }
    )";

    static const char* s_FragmentShaderSource = R"(
        #version 330 core
        in vec3 fragColor;
        in vec2 TexCoord;

        out vec4 FragColor;

        uniform sampler2D u_Texture;

        void main() {
            FragColor = texture(u_Texture, TexCoord) * vec4(fragColor, 1.0);
        }
    )";

    bool Init() {
        s_Shader = new Shader(s_VertexShaderSource, s_FragmentShaderSource);
        s_Texture = new Texture(64, 64);
        s_Batcher = new SpriteBatcher();
        s_DrawCalls = 0;
        return true;
    }

    void Shutdown() {
        delete s_Batcher;
        delete s_Texture;
        delete s_Shader;

        s_Batcher = nullptr;
        s_Texture = nullptr;
        s_Shader = nullptr;
        s_DrawCalls = 0;
    }

    OrthographicCamera& GetCamera() {
        return s_Camera;
    }

    void ResetDrawCalls() {
        s_DrawCalls = 0;
    }

    int GetDrawCalls() {
        int pending = (s_Batcher && s_Batcher->HasPendingDraws()) ? 1 : 0;
        return s_DrawCalls + pending;
    }

    void IncrementDrawCalls() {
        s_DrawCalls++;
    }

    void BeginBatch(float viewportWidth, float viewportHeight) {
        s_DrawCalls = 0;

        if (s_Batcher && s_Shader && s_Texture) {
            s_Batcher->BeginBatch(*s_Shader, *s_Texture);
            std::array<float, 16> proj = s_Camera.GetMatrix(viewportWidth, viewportHeight);
            s_Shader->Bind();
            s_Shader->SetUniformMat4("u_ViewProjection", proj.data());
        }
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3],
                  float pivotX, float pivotY, float texID) {
        if (s_Batcher) {
            s_Batcher->DrawQuad(x, y, width, height, color, pivotX, pivotY, texID);
        }
    }

    void EndBatch() {
        if (s_Batcher) {
            s_Batcher->Flush();
        }
    }
}
