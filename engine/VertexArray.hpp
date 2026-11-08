#pragma once
#include <glad/gl.h>
#include "VertexBuffer.hpp"

class VertexArray {
private:
    GLuint m_RendererID;

public:
    VertexArray() {
        glGenVertexArrays(1, &m_RendererID);
    }

    ~VertexArray() {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void Bind() const {
        glBindVertexArray(m_RendererID);
    }

    void Unbind() const {
        glBindVertexArray(0);
    }

    // Specifies layout for a standard interleaved vertex format (e.g., Position + Color)
    void AddBuffer(const VertexBuffer& vb) {
        Bind();
        vb.Bind();

        // Attribute 0: Position (2 floats)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attribute 1: Color (3 floats)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
};
