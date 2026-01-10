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

    ShaderManager* getShaderManager() const { return m_shaderManager.get(); }
    MeshManager* getMeshManager() const { return m_meshManager.get(); }
    TextureManager* getTextureManager() const { return m_textureManager.get(); }

    Scene* getCurrentScene();
    Camera* getCurrentCamera();

private:
    static constexpr std::array<std::pair<std::string_view, std::string_view>, 2> SCENE_LIST = {{
        {"Test Scene 1", "test_scene_1.yaml"},
        {"Test Scene 2", "test_scene_2.yaml"}
    }};

    const char* m_engineName;
    bool m_isRunning = true;
    const unsigned int m_screenWidth;
    const unsigned int m_screenHeight;
    GLFWwindow* m_window = nullptr;

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<MeshManager> m_meshManager;
    std::unique_ptr<TextureManager> m_textureManager;

    const int m_targetFPS = 60;
    std::unique_ptr<Timer> m_timer;

    std::unique_ptr<DebugWindow> m_debugWindow;

    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    std::string m_currentSceneName;

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

    void setupCameraCallbacks();

    void loadResources();
    std::unique_ptr<ShaderManager> loadShaders();
    std::unique_ptr<MeshManager> loadMeshes();
    std::unique_ptr<TextureManager> loadTextures();

    void createScene(const std::string& sceneName, const std::string& scenePath);
    void createScenes();
    void switchScene(const std::string& sceneName);
};