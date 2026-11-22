#include "renderer/gl/RenderContext.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <glad/gl.h>
#include <iostream>

namespace RenderContext {
    static SDL_GLContext s_GLContext = nullptr;
    static bool s_VSyncEnabled = true;

    void ConfigureAttributes() {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    }

    bool Init(SDL_Window* window) {
        s_GLContext = SDL_GL_CreateContext(window);
        if (!s_GLContext) {
            std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << "\n";
            return false;
        }

        SDL_GL_MakeCurrent(window, s_GLContext);

        if (gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0) {
            std::cerr << "Failed to initialize GLAD\n";
            SDL_GL_DeleteContext(s_GLContext);
            s_GLContext = nullptr;
            return false;
        }

        SetBlendEnabled(true);
        SetDepthEnabled(true);
        SetVSync(true);

        int drawW = 0, drawH = 0;
        SDL_GL_GetDrawableSize(window, &drawW, &drawH);
        SetViewport(drawW, drawH);

        std::cout << "BruhEngine (OpenGL) Loaded Successfully!\n"
                  << "Vendor: " << glGetString(GL_VENDOR) << "\n"
                  << "Renderer: " << glGetString(GL_RENDERER) << "\n"
                  << "OpenGL Version: " << glGetString(GL_VERSION) << "\n"
                  << "Shading: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForOpenGL(window, s_GLContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        return true;
    }

    void Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        if (s_GLContext) {
            SDL_GL_DeleteContext(s_GLContext);
            s_GLContext = nullptr;
        }
    }

    void SetViewport(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void Clear(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void SwapBuffers(SDL_Window* window) {
        if (window) SDL_GL_SwapWindow(window);
    }

    void BeginImGuiFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void EndImGuiFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool SetVSync(bool enabled) {
        int interval = enabled ? 1 : 0;
        if (SDL_GL_SetSwapInterval(interval) < 0) {
            std::cout << "[BruhEngine] Warning: SDL_GL_SetSwapInterval(" << interval
                      << ") failed! SDL_GetError(): " << SDL_GetError() << "\n";
            return false;
        }
        s_VSyncEnabled = enabled;
        return true;
    }

    bool IsVSyncEnabled() { return s_VSyncEnabled; }

    void SetBlendEnabled(bool enabled) {
        if (enabled) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }

    bool IsBlendEnabled() { return glIsEnabled(GL_BLEND); }

    void SetDepthEnabled(bool enabled) {
        if (enabled) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        }
    }

    bool IsDepthEnabled() { return glIsEnabled(GL_DEPTH_TEST); }
}
