#pragma once
#include <glad/gl.h>

class VertexBuffer {
private:
    GLuint m_RendererID;

public:
    // Static buffer constructor for fixed geometry
    VertexBuffer(const void* data, unsigned int size) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    // Dynamic buffer constructor for sprite batching
    VertexBuffer(unsigned int size) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    ~VertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void Unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Update buffer data dynamically per frame
    void SetData(const void* data, unsigned int size, unsigned int offset = 0) const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    }
};
