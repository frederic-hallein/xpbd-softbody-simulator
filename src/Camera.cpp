#include "Camera.hpp"

const float MIN_PHI = glm::radians(0.5f);
const float MAX_PHI = glm::radians(88.0f);


glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(
        glm::radians(m_FOV),
        m_aspectRatio,
        m_nearPlane,
        m_farPlane
    );
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
        m_cameraPos,
        m_cameraPos + m_cameraFront,
        m_cameraUp
    );
}

glm::vec3 Camera::getRayDirection(
    double mouseX,
    double mouseY,
    unsigned int screenWidth,
    unsigned int screenHeight
) const
{
    // mouse position -> NDC (-1 to 1)
    float x = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(screenHeight);

    // Create ray in clip space
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    // Transform to eye space
    glm::mat4 projection = getProjectionMatrix();
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Transform to world space
    glm::mat4 view = getViewMatrix();
    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

    return rayWorld;
}

void Camera::setOrbit() {
    m_sphericalRadius = glm::length(m_cameraPos);
    if (m_sphericalRadius > 0.0f) {
        m_sphericalTheta = std::atan2(m_cameraPos.x, m_cameraPos.z);
        m_sphericalPhi = std::acos(m_cameraPos.y / m_sphericalRadius);
    } else {
        m_sphericalTheta = 0.0f;
        m_sphericalPhi = glm::radians(90.0f);
    }
}

void Camera::updateOrbit() {
    if (m_sphericalPhi < MIN_PHI) m_sphericalPhi = MIN_PHI;
    if (m_sphericalPhi > MAX_PHI) m_sphericalPhi = MAX_PHI;

    float sinPhi   = std::sin(m_sphericalPhi);
    float cosPhi   = std::cos(m_sphericalPhi);
    float sinTheta = std::sin(m_sphericalTheta);
    float cosTheta = std::cos(m_sphericalTheta);

    m_cameraPos.x = m_sphericalRadius * sinPhi * sinTheta;
    m_cameraPos.y = m_sphericalRadius * cosPhi;
    m_cameraPos.z = m_sphericalRadius * sinPhi * cosTheta;

    m_cameraFront = glm::normalize(glm::vec3(0.0f) - m_cameraPos);
    m_cameraRight = glm::normalize(glm::cross(m_worldUp, m_cameraFront));
    m_cameraUp    = glm::normalize(glm::cross(m_cameraFront, m_cameraRight));
}

void Camera::updateOrbitAngles(
    float thetaDelta,
    float phiDelta
)
{
    m_sphericalTheta -= thetaDelta;
    m_sphericalPhi -= phiDelta;
    updateOrbit();
}

void Camera::resetPosition() {
    m_cameraPos = m_originalCameraPos;
    m_cameraFront = glm::normalize(glm::vec3(0.0f) - m_cameraPos);
    m_cameraRight = glm::normalize(glm::cross(m_worldUp, m_cameraFront));
    m_cameraUp = glm::normalize(glm::cross(m_cameraFront, m_cameraRight));
    setOrbit();
}

Camera::Camera(
    glm::vec3 cameraPos,
    float aspectRatio,
    GLFWwindow* window
)
    : m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      m_cameraPos(cameraPos),
      m_originalCameraPos(cameraPos),
      m_cameraFront(glm::normalize(glm::vec3(0.0f) - cameraPos)),
      m_cameraRight(glm::cross(m_worldUp, m_cameraFront)),
      m_cameraUp(m_worldUp),
      m_FOV(45.0f),
      m_aspectRatio(aspectRatio),
      m_nearPlane(0.1f),
      m_farPlane(500.0f),
      m_window(window),
      m_scrollSpeed(150.0f),
      m_mouseSensitivity(0.01),
      m_isDragging(false),
      m_lastX(0.0f),
      m_lastY(0.0f)
{
}

void Camera::scrollCallback(
    GLFWwindow* window,
    double xoffset,
    double yoffset
)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    float scrollAmount = camera->m_scrollSpeed * camera->m_deltaTime;
    camera->setPosition(camera->getPosition() + camera->getFront() * static_cast<float>(yoffset) * scrollAmount);
}


void Camera::mouseButtonCallback(
    GLFWwindow* window,
    int button,
    int action,
    int mods
)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (button != GLFW_MOUSE_BUTTON_RIGHT) return;

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (action == GLFW_PRESS) {
        camera->setDragging(true);
        camera->setOrbit();
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        camera->setLastMousePos(mouseX, mouseY);
    } else if (action == GLFW_RELEASE) {
        camera->setDragging(false);
    }
}

void Camera::cursorPosCallback(
    GLFWwindow* window,
    double xpos,
    double ypos
)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera || !camera->isDragging()) return;

    float xoffset = static_cast<float>(xpos - camera->m_lastX);
    float yoffset = static_cast<float>(ypos - camera->m_lastY);

    camera->setLastMousePos(xpos, ypos);
    camera->updateOrbitAngles(xoffset * camera->m_mouseSensitivity, yoffset * camera->m_mouseSensitivity);
}
