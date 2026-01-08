#pragma once

#include <memory>
#include <glad.h>
#include <GLFW/glfw3.h>

#include "ImGuiWindow.hpp"
#include "Scene.hpp"
#include "Timer.hpp"

class PhysicsEngine
{
public:
    PhysicsEngine(
        const char* engineName,
        int screenWidth,
        int screenHeight
    );

    bool isRunning() const { return m_isRunning; }
    void handleEvents();
    void render();
    void close();

private:
    bool m_isRunning = true;
    int unsigned m_screenWidth;
    int unsigned m_screenHeight;
    GLFWwindow* m_window;

    const int m_targetFPS = 60;
    std::unique_ptr<Timer> m_timer;

    std::unique_ptr<DebugWindow> m_debugWindow;
    std::unique_ptr<Scene> m_scene;
};