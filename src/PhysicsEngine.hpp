#pragma once

#include <memory>
#include <glad.h>
#include <GLFW/glfw3.h>

#include "ImGuiWindow.hpp"
#include "SceneManager.hpp"
#include "Timer.hpp"

class PhysicsEngine
{
public:
    PhysicsEngine(
        const char* engineName,
        const unsigned int screenWidth,
        const unsigned int screenHeight
    );
    ~PhysicsEngine();

    bool isRunning() const { return m_isRunning; }
    void handleEvents();
    void update();
    void render();
    void close();

    ShaderManager* getShaderManager() const { return m_shaderManager.get(); }
    MeshManager* getMeshManager() const { return m_meshManager.get(); }
    TextureManager* getTextureManager() const { return m_textureManager.get(); }

    SceneManager* getSceneManager() const { return m_sceneManager.get(); }
    void switchScene(const std::string& sceneName);

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

    void loadResources();
    std::unique_ptr<ShaderManager> loadShaders();
    std::unique_ptr<MeshManager> loadMeshes();
    std::unique_ptr<TextureManager> loadTextures();

    void processInput();

private:
    const char* m_engineName;
    bool m_isRunning = true;
    const unsigned int m_screenWidth;
    const unsigned int m_screenHeight;
    GLFWwindow* m_window = nullptr;

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<MeshManager> m_meshManager;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<SceneManager> m_sceneManager;

    const int m_targetFPS;
    std::unique_ptr<Timer> m_timer;

    std::unique_ptr<DebugWindow> m_debugWindow;
};