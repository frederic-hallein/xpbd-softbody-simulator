#include "logger.hpp"
#include "PhysicsEngine.hpp"

const std::string SCENES_PATH = "../scenes/";
const std::string RESOURCE_PATH = "../res/";

std::unique_ptr<ShaderManager> PhysicsEngine::loadShaders()
{
    logger::info(" - Loading shaders...");
    auto shaderManager = std::make_unique<ShaderManager>();
    std::vector<std::unique_ptr<Shader>> shaders;

    const std::vector<std::tuple<const char*, const char*, const char*>> shaderData = {
        {"vertexNormal", "vertexNormal.vsh", "vertexNormal.fsh"},
        {"faceNormal", "faceNormal.vsh", "faceNormal.fsh"},
        {"platform", "platform.vsh", "platform.fsh"},
        {"dirtblock", "dirtblock.vsh", "dirtblock.fsh"},
        {"sphere", "sphere.vsh", "sphere.fsh"},
    };

    for (const auto& [name, vsh, fsh] : shaderData) {
        std::string vshPath = std::string(RESOURCE_PATH) + "shaders/" + vsh;
        std::string fshPath = std::string(RESOURCE_PATH) + "shaders/" + fsh;
        try {
            shaders.push_back(std::make_unique<Shader>(name, vshPath.c_str(), fshPath.c_str()));
            logger::info("  - Loaded '{}' shader successfully", name);
        } catch (const std::exception& e) {
            logger::error("- Failed to load '{}' shader: {}", name, e.what());
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

    const std::vector<std::tuple<const char*, const char*>> meshData = {
        {"cube", "cube.obj"},
        {"sphere", "sphere.obj"},
    };

    for (const auto& [name, filename] : meshData) {
        std::string meshPath = std::string(RESOURCE_PATH) + "meshes/" + filename;
        try {
            meshes.push_back(std::make_unique<Mesh>(name, meshPath.c_str()));
            logger::info("  - Loaded '{}' mesh successfully", name);
        } catch (const std::exception& e) {
            logger::error("-Failed to load mesh '{}' : {}", name, e.what());
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

    const std::vector<std::tuple<const char*, const char*>> textureData = {
        {"dirtblock", "dirtblock.jpg"}
    };

    for (const auto& [name, filename] : textureData) {
        std::string texturePath = std::string(RESOURCE_PATH) + "textures/" + filename;
        try {
            textures.push_back(std::make_unique<Texture>(name, texturePath.c_str()));
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
        logger::error("Scene '{}' not found", sceneName);
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

void PhysicsEngine::setupCameraCallbacks()
{
    Camera* camera = getCurrentCamera();
    if (!camera) return;

    glfwSetWindowUserPointer(m_window, camera); // stores camera pointer
    glfwSetScrollCallback(m_window, Camera::scrollCallback);
    glfwSetMouseButtonCallback(m_window, Camera::mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, Camera::cursorPosCallback);
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

    // create resource manager
    // auto shaderManager = std::make_unique<ShaderManager>();
    // auto meshManager = std::make_unique<MeshManager>();
    // auto textureManager = std::make_unique<TextureManager>();

    // create debug window
    m_debugWindow = std::make_unique<DebugWindow>(m_window, "#version 330");

    // load resources
    loadResources();

    // create scene
    createScenes();

    // select first scene
    switchScene("Test Scene 1");

    setupCameraCallbacks();
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
            m_debugWindow->update(m_timer->frameDuration, *currentScene); // TODO : debug broken ImGui
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