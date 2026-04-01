#include "../include/Core/Engine.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Create and initialize the game engine
        Engine engine;
        
        if (!engine.init()) {
            std::cerr << "Failed to initialize engine\n";
            return 1;
        }

        // Run the main game loop
        engine.run();

        // Clean exit
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error\n";
        return 1;
    }
}
