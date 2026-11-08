#include "engine/bruh.hpp"
#include <iostream>
#include <functional>

// Forward declarations of engine API functions
using namespace BruhEngine;

// Test
float squarePosX = 0.0f;

int main() {
    if (!Init("Hello world", 400, 400)) {
        return -1;
    }

    // Fixed update callback
    auto updateLogic = [](float fixedDt) {
        squarePosX += 0.5f * fixedDt;

        // Update window title every frame (or rate-limited)
        std::string title = "BruhEngine | squarePosX: " + std::to_string(squarePosX);
        SetWindowTitle(title);
    };

    // Run
    Run(updateLogic, nullptr);

    Shutdown();
    return 0;
}
