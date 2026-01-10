#include "logger.hpp"
#include "PhysicsEngine.hpp"

PhysicsEngine::PhysicsEngine(
    const char* engineName,
    const unsigned int screenWidth,
    const unsigned int screenHeight
)
    : m_engineName(engineName),
      m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    logger::debug("Running in DEBUG mode");
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
    auto shaderManager = std::make_unique<ShaderManager>();
    auto meshManager = std::make_unique<MeshManager>();
    auto textureManager = std::make_unique<TextureManager>();

    // create scene
    m_scene = std::make_unique<Scene>(
        "test_scene.yaml",
        m_window,
        screenWidth,
        screenHeight
    );

    // create debug window
    m_debugWindow = std::make_unique<DebugWindow>(m_window, "#version 330");
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

        m_scene->update(m_timer->getDeltaTime());
        m_scene->render();

        m_debugWindow->newFrame();
        m_debugWindow->update(m_timer->frameDuration, *m_scene);
        m_debugWindow->render();

        glfwSwapBuffers(m_window);
        glfwPollEvents();

        m_timer->capFrameRate(m_targetFPS);
    }
}

void PhysicsEngine::close()
{
    m_debugWindow->close();
    m_scene->clear();
    glfwTerminate();

    logger::info("{} closed successfully", m_engineName);
}