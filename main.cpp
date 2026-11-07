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

        // Rate-limit console prints to every 0.5 seconds
        static float printTimer = 0.0f;
        printTimer += fixedDt;
        if (printTimer >= 0.5f) {
            std::cout << "squarePosX: " << squarePosX << "\n";
            printTimer = 0.0f;
        }
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
