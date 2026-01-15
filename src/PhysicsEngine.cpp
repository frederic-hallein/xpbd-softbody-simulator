// #include <backends/imgui_impl_glfw.h>

#include "logger.hpp"
#include "ResourceConfig.hpp"
#include "PhysicsEngine.hpp"

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

PhysicsEngine::PhysicsEngine(
    const char* engineName,
    const unsigned int screenWidth,
    const unsigned int screenHeight
)
    : m_engineName(engineName),
      m_screenWidth(screenWidth),
      m_screenHeight(screenHeight),
      m_targetFPS(60.0f)
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
    const char* glslVersion = "#version 330";
    m_debugWindow = std::make_unique<DebugWindow>(m_window, glslVersion);

    // load resources
    loadResources();

    // create scene manager
    m_sceneManager = std::make_unique<SceneManager>(
        m_window,
        screenWidth,
        screenHeight,
        m_shaderManager.get(),
        m_meshManager.get(),
        m_textureManager.get()
    );

    // create and select first scene
    m_sceneManager->createScenes();
    m_sceneManager->switchScene(std::string(SCENE_LIST[0].first));
};

PhysicsEngine::~PhysicsEngine()
{
}

void PhysicsEngine::handleEvents()
{
    if (glfwWindowShouldClose(m_window))
    {
        m_isRunning = false;
        logger::info("Closing {}...", m_engineName);
    }
}

void PhysicsEngine::processInput()
{
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);

    Camera* camera = m_sceneManager->getCurrentCamera();
    if (camera) {
        camera->setDeltaTime(m_timer->getDeltaTime());
    }

    // TODO : update camera controls
    // Camera* camera = m_sceneManager->getCurrentCamera();
    // if (camera) {
    //     if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    //         camera->processKeyboard(FORWARD, m_timer->getDeltaTime());
    //     if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    //         camera->processKeyboard(BACKWARD, m_timer->getDeltaTime());
    //     if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    //         camera->processKeyboard(LEFT, m_timer->getDeltaTime());
    //     if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    //         camera->processKeyboard(RIGHT, m_timer->getDeltaTime());
    // }
}

void PhysicsEngine::update()
{
    processInput();
    m_timer->startFrame();

    Scene* currentScene = m_sceneManager->getCurrentScene();
    if (currentScene) {
        currentScene->update(m_timer->getDeltaTime());
    }
}

void PhysicsEngine::render()
{
    Scene* currentScene = m_sceneManager->getCurrentScene();
    if (currentScene) {
        currentScene->render();

        m_debugWindow->newFrame();
        m_debugWindow->update(m_timer->frameDuration, *currentScene, *m_sceneManager);
        m_debugWindow->render();
    }

    glfwSwapBuffers(m_window);
    glfwPollEvents();
    m_timer->capFrameRate(m_targetFPS);
}

void PhysicsEngine::close()
{
    m_debugWindow->close();
    m_sceneManager->clearScenes();

    glfwTerminate();

    logger::info("{} closed successfully", m_engineName);
}