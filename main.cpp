#include "engine/bruh.cpp"

int main(int argc, char* argv[]) {
    if (!BruhEngine::Init("Hello world", 400, 400)) {
        return -1;
    }

    // Main app loop
    while (BruhEngine::IsRunning()) {
        BruhEngine::ProcessEvents();
        BruhEngine::Update();
        BruhEngine::Render();
    }

    BruhEngine::Shutdown();
    return 0;
}
