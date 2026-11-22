#pragma once
#include "renderer2d/OrthographicCamera.hpp"

namespace Renderer2D {
    bool Init();
    void Shutdown();

    OrthographicCamera& GetCamera();

    void ResetDrawCalls();
    int GetDrawCalls();
    void IncrementDrawCalls();

    void BeginBatch(float viewportWidth, float viewportHeight);
    void DrawQuad(float x, float y, float width, float height, const float color[3],
                  float pivotX = 0.5f, float pivotY = 0.5f, float texID = 0.0f);
    void EndBatch();
}
