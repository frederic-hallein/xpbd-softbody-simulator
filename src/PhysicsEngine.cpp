#include <backends/imgui_impl_glfw.h>

#include "logger.hpp"
#include "ResourceConfig.hpp"
#include "PhysicsEngine.hpp"

const std::string SCENES_PATH = "../scenes/";
const std::string RESOURCE_PATH = "../res/";


std::unique_ptr<ShaderManager> PhysicsEngine::loadShaders()
{
    logger::info(" - Loading shaders...");
    auto shaderManager = std::make_unique<ShaderManager>();
    std::vector<std::unique_ptr<Shader>> shaders;

    for (const auto& [name, vsh, fsh] : SHADER_DATA) {
        std::string vshPath = std::string(RESOURCE_PATH) + "shaders/" + std::string(vsh);
        std::string fshPath = std::string(RESOURCE_PATH) + "shaders/" + std::string(fsh);
        try {
            shaders.push_back(std::make_unique<Shader>(std::string(name).c_str(), vshPath.c_str(), fshPath.c_str()));
            logger::info("  - Loaded '{}' shader successfully", name);
        } catch (const std::exception& e) {
            logger::error("Failed to load '{}' shader: {}", name, e.what());
        }
    }

    shaderManager->addResources(std::move(shaders));
    return shaderManager;
}

std::unique_ptr<MeshManager> PhysicsEngine::loadMeshes()
{
    logger::info(" - Loading meshes...");
    auto meshManager = std::make_unique<MeshManager>();
    std::vector<std::unique_ptr<Mesh>> meshes;

    for (const auto& [name, filename] : MESH_DATA) {
        std::string meshPath = std::string(RESOURCE_PATH) + "meshes/" + std::string(filename);
        try {
            meshes.push_back(std::make_unique<Mesh>(std::string(name).c_str(), meshPath.c_str()));
            logger::info("  - Loaded '{}' mesh successfully", name);
        } catch (const std::exception& e) {
            logger::error("Failed to load mesh '{}' : {}", name, e.what());
        }
    }

    meshManager->addResources(std::move(meshes));
    return meshManager;
}

std::unique_ptr<TextureManager> PhysicsEngine::loadTextures()
{
    logger::info(" - Loading textures...");
    auto textureManager = std::make_unique<TextureManager>();
    std::vector<std::unique_ptr<Texture>> textures;

    for (const auto& [name, filename] : TEXTURE_DATA) {
        std::string texturePath = std::string(RESOURCE_PATH) + "textures/" + std::string(filename);
        try {
            textures.push_back(std::make_unique<Texture>(std::string(name).c_str(), texturePath.c_str()));
            logger::info("  - Loaded '{}' texture successfully", name);
        } catch (const std::exception& e) {
            logger::error("- Failed to load texture '{}' : {}", name, e.what());
        }
    }

    textureManager->addResources(std::move(textures));
    return textureManager;
}

void PhysicsEngine::loadResources()
{
    logger::info("Loading resources...");
    m_shaderManager = loadShaders();
    m_meshManager = loadMeshes();
    m_textureManager = loadTextures();
    logger::info("Loaded resources successfully");
}

void PhysicsEngine::createScene(const std::string& sceneName, const std::string& sceneFilename)
{
    const std::string scenePath = "../scenes/" + sceneFilename;
    try {
        m_scenes[sceneName] = std::make_unique<Scene>(
            m_window,
            m_screenWidth,
            m_screenHeight,
            m_shaderManager.get(),
            m_meshManager.get(),
            m_textureManager.get()
        );
        m_scenes[sceneName]->loadSceneConfig(scenePath);
        logger::info(" - Created '{}' scene successfully", sceneName);
    } catch (const std::exception& e) {
        logger::error("Failed to load scene '{}': {}", sceneName, e.what());
    }
}

void PhysicsEngine::createScenes()
{
    logger::info("Creating scenes...");
    for (const auto& [sceneName, sceneFilename] : SCENE_LIST) {
        createScene(std::string(sceneName), std::string(sceneFilename));
    }
}

void PhysicsEngine::switchScene(const std::string& sceneName)
{
    auto it = m_scenes.find(sceneName);
    if (it == m_scenes.end()) {
        logger::error("Scene '{}' not found", sceneName); // TODO : better handle scene not found error
        return;
    }

    m_currentSceneName = sceneName;
    setupCameraCallbacks();
    logger::info("Switched to scene: {}", sceneName);
}


Scene* PhysicsEngine::getCurrentScene()
{
    auto it = m_scenes.find(m_currentSceneName);
    if (it == m_scenes.end()) {
        return nullptr;
    }
    return it->second.get();
}

Camera* PhysicsEngine::getCurrentCamera()
{
    Scene* scene = getCurrentScene();
    if (!scene) return nullptr;
    return scene->getCamera();
}

// Forward to ImGui first, then to Camera
static void GlfwScrollDispatcher(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    Camera::scrollCallback(window, xoffset, yoffset);
}

static void GlfwMouseButtonDispatcher(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    Camera::mouseButtonCallback(window, button, action, mods);
}

static void GlfwCursorPosDispatcher(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    Camera::cursorPosCallback(window, xpos, ypos);
}

void PhysicsEngine::setupCameraCallbacks()
{
    Camera* camera = getCurrentCamera();
    if (!camera) return;

    glfwSetWindowUserPointer(m_window, camera);

    glfwSetScrollCallback(m_window, GlfwScrollDispatcher);
    glfwSetMouseButtonCallback(m_window, GlfwMouseButtonDispatcher);
    glfwSetCursorPosCallback(m_window, GlfwCursorPosDispatcher);

    glfwSetKeyCallback(m_window, ImGui_ImplGlfw_KeyCallback);
    glfwSetCharCallback(m_window, ImGui_ImplGlfw_CharCallback);
}

PhysicsEngine::PhysicsEngine(
    const char* engineName,
    const unsigned int screenWidth,
    const unsigned int screenHeight
)
    : m_engineName(engineName),
      m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    logger::debug("--- Running in DEBUG mode ---");
    logger::info("Initializing: {}", engineName);

    // init GLFW window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(screenWidth, screenHeight, engineName, NULL, NULL);
    if(!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        throw std::runtime_error("Failed to load GLAD");
    }

    logger::info("GLFW window created");


    // create timer
    m_timer = std::make_unique<Timer>();

    // adjust viewport when window resizing
    framebufferSizeCallback(m_window, screenWidth, screenHeight);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    // create debug window
    m_debugWindow = std::make_unique<DebugWindow>(m_window, "#version 330");

    // load resources
    loadResources();


    // create and select first scene
    createScenes();
    switchScene("Test Scene 1"); // TODO : couple with ImGui
};

void PhysicsEngine::handleEvents()
{
    if (glfwWindowShouldClose(m_window))
    {
        m_isRunning = false;
        logger::info("Closing {}", m_engineName);
    }
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void PhysicsEngine::render()
{
    while (!glfwWindowShouldClose(m_window))
    {
        processInput(m_window);

        m_timer->startFrame();

        Scene* currentScene = getCurrentScene();
        if (currentScene) {
            currentScene->update(m_timer->getDeltaTime());
            currentScene->render();

            m_debugWindow->newFrame();
            m_debugWindow->update(m_timer->frameDuration, *currentScene);
            m_debugWindow->render();
        }

        glfwSwapBuffers(m_window);
        glfwPollEvents();

        m_timer->capFrameRate(m_targetFPS);
    }
}

void PhysicsEngine::close()
{
    m_debugWindow->close();
    for (auto& [name, scene] : m_scenes) {
        scene->clear();
    }
    glfwTerminate();

    logger::info("{} closed successfully", m_engineName);
}