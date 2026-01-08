#pragma once

#include <glm/glm.hpp>

#include "Scene.hpp"

class ImGuiWindow
{
public:
    ImGuiWindow() = default;
    ImGuiWindow(
        GLFWwindow* window,
        const char* glslVersion
    );

    void newFrame();
    void update();
    void render();
    void close();

private:

};

class DebugWindow : public ImGuiWindow
{
public:
    DebugWindow() = default;
    DebugWindow(
        GLFWwindow* window,
        const char* glslVersion
    );

    void update(
        int frameDuration,
        Scene& scene
    );
};