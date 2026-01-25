#include "logger.hpp"
#include "PhysicsEngine.hpp"

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {
    try {
        PhysicsEngine physicsEngine(
            "XPBD Softbody Implementation",
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
        while (physicsEngine.isRunning()) {
            physicsEngine.handleEvents();
            physicsEngine.update();
            physicsEngine.render();
        }

        physicsEngine.close();
    } catch (const std::exception& e) {
        logger::error("Engine failed to initialize: {}", e.what());
        return 1;
    }

    return 0;
}
