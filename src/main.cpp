#include "logger.hpp"
#include "PhysicsEngine.hpp"

const int unsigned SCREEN_WIDTH = 1080;
const int unsigned SCREEN_HEIGHT = 720;

int main(int argc, char* argv[])
{
    try {
        PhysicsEngine physicsEngine(
            "XPBD Softbody Implementation",
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
        while (physicsEngine.isRunning())
        {
            physicsEngine.handleEvents();
            physicsEngine.render();
        }

        physicsEngine.close();
    } catch (const std::exception& e) {
        logger::error("Engine failed to initialize: {}", e.what());
        return 1;
    }

    return 0;
}
