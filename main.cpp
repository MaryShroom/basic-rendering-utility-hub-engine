#include "engine/bruh.cpp"
#include <iostream>

// Test
float squarePosX = 0.0f;

int main(int argc, char* argv[]) {
    if (!BruhEngine::Init("Hello world", 400, 400)) {
        return -1;
    }

    // Fixed update callback
    auto updateLogic = [](float fixedDt) {
        squarePosX += 0.5f * fixedDt;

        // Update window title every frame (or rate-limited)
        std::string title = "BruhEngine | squarePosX: " + std::to_string(squarePosX);
        BruhEngine::SetWindowTitle(title);
    };

    // Render clalback
    auto renderFrame = []() {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // R,G,B,A
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    };

    // Run
    BruhEngine::Run(updateLogic, renderFrame);

    BruhEngine::Shutdown();
    return 0;
}
