#pragma once
#include <glad/gl.h>
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <cstdint>

struct SpriteVertex {
    float Position[2];   // Anchor position (x, y)
    float Color[3];      // RGB tint
    float TexCoords[2];  // UV coordinates
    float TexID;         // Texture slot ID
    float Pivot[2];      // Pivot offset (e.g., 0.5, 0.5 for center)
};

class SpriteBatcher {
private:
    static const uint32_t MaxSprites = 10000;
    static const uint32_t MaxVertices = MaxSprites * 4;
    static const uint32_t MaxIndices = MaxSprites * 6;

    VertexArray* m_VertexArray = nullptr;
    VertexBuffer* m_VertexBuffer = nullptr;
    IndexBuffer* m_IndexBuffer = nullptr;

    uint32_t m_IndexCount = 0;
    SpriteVertex* m_VertexBufferBase = nullptr;
    SpriteVertex* m_VertexBufferPtr = nullptr;

    Shader* m_ActiveShader = nullptr;
    Texture* m_ActiveTexture = nullptr;

public:
    SpriteBatcher() {
        m_VertexBufferBase = new SpriteVertex[MaxVertices];

        m_VertexArray = new VertexArray();
        m_VertexBuffer = new VertexBuffer(MaxVertices * sizeof(SpriteVertex));

        m_VertexArray->Bind();
        m_VertexBuffer->Bind();

        // Attribute 0: Position (2 floats)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, Position));

        // Attribute 1: Color (3 floats)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, Color));

        // Attribute 2: Texture Coordinates (2 floats)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, TexCoords));

        // Attribute 3: Texture ID (1 float)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, TexID));

        // Attribute 3: Pivot (2 floats)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, Pivot));

        // Attribute 4: Texture ID (1 float) - shifted from attribute 3
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, TexID));

        // Generate static index buffer once upfront for all quads
        uint32_t* indices = new uint32_t[MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices; i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
            offset += 4;
        }

        m_IndexBuffer = new IndexBuffer(indices, MaxIndices);
        delete[] indices;
    }

    ~SpriteBatcher() {
        delete m_IndexBuffer;
        delete m_VertexBuffer;
        delete m_VertexArray;
        delete[] m_VertexBufferBase;
    }

    void BeginBatch(Shader& shader, Texture& texture) {
        m_VertexBufferPtr = m_VertexBufferBase;
        m_IndexCount = 0;
        m_ActiveShader = &shader;
        m_ActiveTexture = &texture;
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3], float pivotX = 0.5f, float pivotY = 0.5f, float texID = 0.0f) {
        if (m_IndexCount >= MaxIndices) Flush(); // Prevent overflow

        // Compute local offset based on pivot dimensions
        float xMin = -width * pivotX;
        float xMax = width * (1.0f - pivotX);
        float yMin = -height * pivotY;
        float yMax = height * (1.0f - pivotY);

        // Map local offsets to world/NDC positions around (x, y)
        float x0 = x + xMin; // Bottom-Left / Top-Left X
        float x1 = x + xMax; // Bottom-Right / Top-Right X
        float y0 = y + yMin; // Bottom-Left / Bottom-Right Y
        float y1 = y + yMax; // Top-Left / Top-Right Y

        // Bottom-Left
        m_VertexBufferPtr->Position[0] = x0;
        m_VertexBufferPtr->Position[1] = y0;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 0.0f; m_VertexBufferPtr->TexCoords[1] = 0.0f;
        m_VertexBufferPtr->Pivot[0] = pivotX;
        m_VertexBufferPtr->Pivot[1] = pivotY;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        // Bottom-Right
        m_VertexBufferPtr->Position[0] = x1;
        m_VertexBufferPtr->Position[1] = y0;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 1.0f; m_VertexBufferPtr->TexCoords[1] = 0.0f;
        m_VertexBufferPtr->Pivot[0] = pivotX;
        m_VertexBufferPtr->Pivot[1] = pivotY;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        // Top-Right
        m_VertexBufferPtr->Position[0] = x1;
        m_VertexBufferPtr->Position[1] = y1;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 1.0f; m_VertexBufferPtr->TexCoords[1] = 1.0f;
        m_VertexBufferPtr->Pivot[0] = pivotX;
        m_VertexBufferPtr->Pivot[1] = pivotY;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        // Top-Left
        m_VertexBufferPtr->Position[0] = x0;
        m_VertexBufferPtr->Position[1] = y1;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 0.0f; m_VertexBufferPtr->TexCoords[1] = 1.0f;
        m_VertexBufferPtr->Pivot[0] = pivotX;
        m_VertexBufferPtr->Pivot[1] = pivotY;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        m_IndexCount += 6;
    }

    void EndBatch() {
        // Upload any remaining data in the buffer before final flush
        if (m_IndexCount > 0) {
            uint32_t dataSize = (uint32_t)((uint8_t*)m_VertexBufferPtr - (uint8_t*)m_VertexBufferBase);
            m_VertexBuffer->SetData(m_VertexBufferBase, dataSize);
        }
    }

    void Flush() {
        if (m_IndexCount == 0) return;

        // Upload current buffer chunk
        uint32_t dataSize = (uint32_t)((uint8_t*)m_VertexBufferPtr - (uint8_t*)m_VertexBufferBase);
        m_VertexBuffer->SetData(m_VertexBufferBase, dataSize);

        m_ActiveTexture->Bind(0);
        m_ActiveShader->Bind();
        m_ActiveShader->SetUniform1f("u_Texture", 0);

        m_VertexArray->Bind();
        m_IndexBuffer->Bind();
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);

        BruhEngine::IncrementDrawCalls();

        // Reset index count for next batch chunk
        m_VertexBufferPtr = m_VertexBufferBase;
        m_IndexCount = 0;
    }

    bool HasPendingDraws() const {
        return m_IndexCount > 0;
    }
};
