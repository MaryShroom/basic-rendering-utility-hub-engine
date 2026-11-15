#pragma once
#include <array>
#include <algorithm>

struct OrthographicCamera {
    // World position of the camera center
    float posX = 0.0f;
    float posY = 0.0f;

    // Zoom factor
    float zoom = 1.0f;

    // World unit (700x700)
    float baseWidth = 700.0f;
    float baseHeight = 700.0f;

    // Generate a column-major 4x4  orthographic matrix
    std::array<float, 16> GetMatrix(float windowWidth, float windowHeight) {
        // Divide-by-zero checker
        float aspect = (windowHeight > 0.0f) ? (windowWidth / windowHeight) : 1.0f;

        // Keep world height fixed and scale width dynamically based on window aspect ratio
        float halfH = (baseHeight / 2.0f) / zoom;
        float halfW = halfH * aspect;

        float left = posX - halfW;
        float right = posX + halfW;
        float bottom = posY - halfH;
        float top = posY + halfH;

        // Near = -1.0, Far = 1.0 in 2D
        float zNear = -1.0f;
        float zFar = 1.0f;

        // Standard column-major 4.4 matrix
        std::array<float, 16> mat = {0.0f}; // Zero-init

        mat[0] = 2.0f / (right - left);
        mat[5] = 2.0f / (top - bottom);
        mat[10] = -2.0f / (zFar - zNear);

        mat[12] = -(right + left) / (right - left);
        mat[13] = -(top + bottom) / (top - bottom);
        mat[14] = -(zFar + zNear) / (zFar - zNear);
        mat[15] = 1.0f;

        return mat;
    }
};
