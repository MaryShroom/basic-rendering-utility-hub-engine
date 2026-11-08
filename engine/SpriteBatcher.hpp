#pragma once
#include <glad/gl.h>
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <cstdint>

struct SpriteVertex {
    float Position[2];
    float Color[3];
    float TexCoords[2];
    float TexID;
};

class SpriteBatcher {
private:
    static const uint32_t MaxSprites = 100000;
    static const uint32_t MaxVertices = MaxSprites * 4;
    static const uint32_t MaxIndices = MaxSprites * 6;

    VertexArray* m_VertexArray = nullptr;
    VertexBuffer* m_VertexBuffer = nullptr;
    IndexBuffer* m_IndexBuffer = nullptr;

    uint32_t m_IndexCount = 0;
    SpriteVertex* m_VertexBufferBase = nullptr;
    SpriteVertex* m_VertexBufferPtr = nullptr;

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

    void BeginBatch() {
        m_VertexBufferPtr = m_VertexBufferBase;
        m_IndexCount = 0;
    }

    void DrawQuad(float x, float y, float width, float height, const float color[3], float texID = 0.0f) {
        if (m_IndexCount >= MaxIndices) return; // Prevent overflow

        // Bottom-Left
        m_VertexBufferPtr->Position[0] = x;
        m_VertexBufferPtr->Position[1] = y;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 0.0f; m_VertexBufferPtr->TexCoords[1] = 0.0f;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        // Bottom-Right
        m_VertexBufferPtr->Position[0] = x + width;
        m_VertexBufferPtr->Position[1] = y;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 1.0f; m_VertexBufferPtr->TexCoords[1] = 0.0f;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        // Top-Right
        m_VertexBufferPtr->Position[0] = x + width;
        m_VertexBufferPtr->Position[1] = y + height;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 1.0f; m_VertexBufferPtr->TexCoords[1] = 1.0f;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        // Top-Left
        m_VertexBufferPtr->Position[0] = x;
        m_VertexBufferPtr->Position[1] = y + height;
        m_VertexBufferPtr->Color[0] = color[0]; m_VertexBufferPtr->Color[1] = color[1]; m_VertexBufferPtr->Color[2] = color[2];
        m_VertexBufferPtr->TexCoords[0] = 0.0f; m_VertexBufferPtr->TexCoords[1] = 1.0f;
        m_VertexBufferPtr->TexID = texID;
        m_VertexBufferPtr++;

        m_IndexCount += 6;
    }

    void EndBatch() {
        uint32_t dataSize = (uint32_t)((uint8_t*)m_VertexBufferPtr - (uint8_t*)m_VertexBufferBase);
        m_VertexBuffer->SetData(m_VertexBufferBase, dataSize);
    }

    void Flush(Shader& shader, Texture& texture) {
        texture.Bind(0);
        shader.Bind();
        shader.SetUniform1f("u_Texture", 0);

        m_VertexArray->Bind();
        m_IndexBuffer->Bind();
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);

        BruhEngine::IncrementDrawCalls();
    }
};
