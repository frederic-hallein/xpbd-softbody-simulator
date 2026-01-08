#include "PhysicsEngine.hpp"

const unsigned int SCREEN_WIDTH = 1080;
const unsigned int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[])
{
    PhysicsEngine physicsEngine("XPBD Softbody Implementation", SCREEN_WIDTH, SCREEN_HEIGHT);
    while (physicsEngine.isRunning())
    {
        physicsEngine.handleEvents();
        physicsEngine.render();
    }
    physicsEngine.close();
    return 0;
}
