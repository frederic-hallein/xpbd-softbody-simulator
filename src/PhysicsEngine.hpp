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
        const unsigned int screenWidth,
        const unsigned int screenHeight
    );

    bool isRunning() const { return m_isRunning; }
    void handleEvents();
    void render();
    void close();

private:
    bool m_isRunning = true;
    const unsigned int m_screenWidth;
    const unsigned int m_screenHeight;
    GLFWwindow* m_window;

    const int m_targetFPS = 60;
    std::unique_ptr<Timer> m_timer;

    std::unique_ptr<DebugWindow> m_debugWindow;
    std::unique_ptr<Scene> m_scene;

private:
    const char* m_engineName;
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
};