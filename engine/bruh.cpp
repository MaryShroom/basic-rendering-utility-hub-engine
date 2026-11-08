#include "bruh.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include <SDL.h>
#include <glad/gl.h>
#include <iostream>
#include <functional>
#include <string>

namespace BruhEngine {
    static SDL_Window* window = nullptr;
    static SDL_GLContext glContext = nullptr;
    static bool isRunning = false;

    // Engine-managed OpenGL pipeline resources for the triangle
    static Shader* shader = nullptr;
    static VertexBuffer* vbo = nullptr;
    static VertexArray* vao = nullptr;

    // Minimal vertex and fragment shaders with color interpolation
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 fragColor;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "   fragColor = aColor;\n"
        "}\n";

    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 fragColor;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(fragColor, 1.0);\n"
        "}\n";

    bool Init(const char* title, int width, int height) {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
            return false;
        }

        // OpenGL 3.3 Core Profile
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // Request double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Create window
        window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

        if (!window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }

        // Create the OpenGL context
        glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            std::cerr << "OpenGL context could not be created! Ensure your hardware supports OpenGL 3.3. SDL_Error: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }

        SDL_GL_MakeCurrent(window, glContext);

        // Initialize GLAD v2
        int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
        if (version == 0) {
            std::cerr << "Failed to initialize GLAD\n";
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }

        // Vsync
        if (SDL_GL_SetSwapInterval(1) < 0) {
            std::cerr << "Warning: Unable to set V-Sync! SDL_Error: " << SDL_GetError() << "\n";
        }

        // Get initial drawable size for the viewport
        int drawWidth, drawHeight;
        SDL_GL_GetDrawableSize(window, &drawWidth, &drawHeight);
        glViewport(0, 0, drawWidth, drawHeight);

        // Compile Shaders
        shader = new Shader(vertexShaderSource, fragmentShaderSource);

        // Setup vertex data (VBO & VAO)
        float vertices[] = {
            // Positions    // Colors
             0.0f,  0.5f,   1.0f, 0.0f, 0.0f, // Top Red
            -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // Bottom-Left Green
             0.5f, -0.5f,   0.0f, 0.0f, 1.0f  // Bottom-right Blue
        };

        vbo = new VertexBuffer(vertices, sizeof(vertices));
        vao = new VertexArray();
        vao->AddBuffer(*vbo);

        isRunning = true;
        std::cout << "BruhEngine (OpenGL) Loaded Successfully!\n";
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
        std::cout << "Shading: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
        return true;
    }

    void ProcessEvents(std::function<void(const SDL_Event&)> onEvent = nullptr) {
        // Main loop
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                isRunning = false;
            }
            // Window events
            else if (e.type == SDL_WINDOWEVENT) {
                // Window resize
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    int drawWidth, drawHeight;
                    SDL_GL_GetDrawableSize(window, &drawWidth, &drawHeight);
                    glViewport(0, 0, drawWidth, drawHeight);
                }
            }
            // Foward keys to main
            if (onEvent) onEvent(e);
        }
    }

    void RenderTriangle() {
        glClearColor(0.1f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->Bind();
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void Run(std::function<void(float fixedDeltaTime)> updateFunc, std::function<void()> renderFunc) {
        const double targetFPS = 60.0;
        const double fixedDt = 1.0 / targetFPS;

        Uint64 lastCounter = SDL_GetPerformanceCounter();
        double accumulator = 0.0;

        while (isRunning) {
            ProcessEvents();

            // Calculate frame time delta
            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 counterFrequency = SDL_GetPerformanceFrequency();
            double frameTime = (double)(currentCounter - lastCounter) / (double)counterFrequency;
            lastCounter = currentCounter;

            // Prevent die if a frame takes too long
            if (frameTime > 0.25) frameTime = 0.25;

            accumulator += frameTime;

            // Fixed-timestep update loop
            while (accumulator >= fixedDt) {
                if (updateFunc) updateFunc((float)fixedDt);
                accumulator -= fixedDt;
            }

            // Render (use engine render if main.cpp did not provide one)
            if (renderFunc) renderFunc();
            else RenderTriangle();

            // Buffer
            SDL_GL_SwapWindow(window);
        }
    }

    void SetWindowTitle(const std::string& title) {
        if (window) SDL_SetWindowTitle(window, title.c_str());
    }

    void GetWindowSize(int* w, int* h) {
        if (window) SDL_GetWindowSize(window, w, h);
    }

    void Shutdown() {
        // GPU resources cleanup
        delete shader;
        delete vbo;
        delete vao;
        shader = nullptr;
        vbo = nullptr;
        vao = nullptr;

        if (glContext) SDL_GL_DeleteContext(glContext);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        std::cout << "BruhEngine Shut Down Cleanly.\n";
    }
}
