#pragma once
#include <glad/gl.h>
#include <vector>
#include <cstdint>

class Texture {
private:
    GLuint m_RendererID;
    int m_Width, m_Height;

public:
    // Generate a procedural checkerboard texture for quick testing
    Texture(int width = 64, int height = 64)
        : m_Width(width), m_Height(height) {

        std::vector<uint8_t> pixels(width * height * 4);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = (y * width + x) * 4;
                bool isWhite = ((x / 8) + (y / 8)) % 2 == 0;
                uint8_t color = isWhite ? 255 : 50;

                pixels[index + 0] = color; // R
                pixels[index + 1] = color; // G
                pixels[index + 2] = color; // B
                pixels[index + 3] = 255;   // A
            }
        }

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~Texture() {
        glDeleteTextures(1, &m_RendererID);
    }

    void Bind(unsigned int slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }

    void Unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
