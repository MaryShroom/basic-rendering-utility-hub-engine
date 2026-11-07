#include <SDL.h>
#include <glad/gl.h>
#include <iostream>

namespace BruhEngine {
    static SDL_Window* window = nullptr;
    static SDL_GLContext glContext = nullptr;
    static bool isRunning = false;

    bool Init(const char* title, int width, int height) {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
            return 1;
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
            return 1;
        }

        // Create the OpenGL context
        glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            std::cerr << "OpenGL context could not be created! Ensure your hardware supports OpenGL 3.3. SDL_Error: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        SDL_GL_MakeCurrent(window, glContext);

        // Initialize GLAD v2
        int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
        if (version == 0) {
            std::cerr << "Failed to initialize GLAD\n";
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // Vsync
        if (SDL_GL_SetSwapInterval(1) < 0) {
            std::cerr << "Warning: Unable to set V-Sync! SDL_Error: " << SDL_GetError() << "\n";
        }

        // Get initial drawable size for the viewport
        int drawWidth, drawHeight;
        SDL_GL_GetDrawableSize(window, &drawWidth, &drawHeight);
        glViewport(0, 0, drawWidth, drawHeight);

        isRunning = true;
        std::cout << "BruhEngine (OpenGL) Loaded Successfully!\n";
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
        std::cout << "Shading: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
        return true;
    }

    void ProcessEvents() {
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
            // Key ESC
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }
        }
    }

    void Update() {
        // Game update logic goes here
    }

    void Render() {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // R,G,B,A
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Buffer
        SDL_GL_SwapWindow(window);
    }

    bool IsRunning() {
        return isRunning;
    }

    void Shutdown() {
        if (glContext) {
            SDL_GL_DeleteContext(glContext);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
        std::cout << "BruhEngine Shut Down Cleanly.\n";
    }
}
