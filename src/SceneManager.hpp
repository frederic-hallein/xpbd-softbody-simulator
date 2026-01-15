#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "Scene.hpp"
#include "ShaderManager.hpp"
#include "MeshManager.hpp"
#include "TextureManager.hpp"

class SceneManager
{
public:
    SceneManager(
        GLFWwindow* window,
        unsigned int screenWidth,
        unsigned int screenHeight,
        ShaderManager* shaderManager,
        MeshManager* meshManager,
        TextureManager* textureManager
    );

    void createScenes();
    void switchScene(const std::string& sceneName);
    Scene* getCurrentScene();
    const std::string& getCurrentSceneName() const { return m_currentSceneName; }
    const std::unordered_map<std::string, std::unique_ptr<Scene>>& getAllScenes() const { return m_scenes; }
    void clearScenes();

    Camera* getCurrentCamera();
    void setupCameraCallbacks();
private:
    void createScene(const std::string& sceneName, const std::string& sceneFilename);

private:
    GLFWwindow* m_window;
    unsigned int m_screenWidth;
    unsigned int m_screenHeight;

    ShaderManager* m_shaderManager;
    MeshManager* m_meshManager;
    TextureManager* m_textureManager;

    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    std::string m_currentSceneName;
};