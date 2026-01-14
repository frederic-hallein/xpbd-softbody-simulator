#pragma once

#include <deque>
#include <glm/glm.hpp>

#include "Scene.hpp"
// #include "SceneManager.hpp"

class ImGuiWindow
{
public:
    ImGuiWindow(
        GLFWwindow* window,
        const char* glslVersion
    );

    void newFrame();
    void update();
    void render();
    void close();
};

class DebugWindow : public ImGuiWindow
{
public:
    DebugWindow(
        GLFWwindow* window,
        const char* glslVersion
    );

    void update(
        int frameDuration,
        Scene& scene
    );

private:
    void displayPerformance(int frameDuration);
    void displayCamera(Camera* camera);
    void displayExternalForces(Scene& scene);
    void displayXPBDParameters(Scene& scene);
    void displaySceneReset(Scene& scene);
    void displayPolygonMode(size_t objectIndex, Object* object);
    void displayObjectPanel(size_t objectIndex, Object* object);
    void displayVertexTransforms(size_t objectIndex, Object* object);
    void displaySceneObjects(Scene& scene);

private:
    std::deque<float> m_fpsHistory;
};
