#pragma once
#include <glad/gl.h>
#include <string>
#include <iostream>

class Shader {
private:
    GLuint m_RendererID;

    GLuint CompileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << "\n";
        }
        return shader;
    }

public:
    Shader(const char* vertexSource, const char* fragmentSource) {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vs);
        glAttachShader(m_RendererID, fs);
        glLinkProgram(m_RendererID);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    ~Shader() {
        glDeleteProgram(m_RendererID);
    }

    void Bind() const {
        glUseProgram(m_RendererID);
    }

    void Unbind() const {
        glUseProgram(0);
    }

    void SetUniform1i(const std::string& name, int value) {
        glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
    }

    void SetUniform2f(const std::string& name, float v0, float v1) {
        glUniform2f(glGetUniformLocation(m_RendererID, name.c_str()), v0, v1);
    }
};
