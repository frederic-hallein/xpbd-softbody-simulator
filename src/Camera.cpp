#include "Camera.hpp"


glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_FOV), m_aspectRatio, m_nearPlane, m_farPlane);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}

glm::vec3 Camera::getRayDirection(double mouseX, double mouseY, unsigned int screenWidth, unsigned int screenHeight) const
{
    // Convert mouse position to normalized device coordinates (-1 to 1)
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

void Camera::setOrbit()
{
    m_orbitRadius = glm::length(m_cameraPos);
    if (m_orbitRadius > 0.0f)
    {
        m_orbitPitch = std::asin(m_cameraPos.y / m_orbitRadius);
        m_orbitYaw = std::atan2(m_cameraPos.x, m_cameraPos.z);
    }
    else
    {
        m_orbitPitch = 0.0f;
        m_orbitYaw = 0.0f;
    }
}

void Camera::updateOrbit()
{
    // Clamp pitch to avoid flipping
    if (m_orbitPitch > glm::radians(89.0f)) m_orbitPitch = glm::radians(89.0f);
    if (m_orbitPitch < glm::radians(-89.0f)) m_orbitPitch = glm::radians(-89.0f);

    m_cameraPos.x = m_orbitRadius * cos(m_orbitPitch) * sin(m_orbitYaw);
    m_cameraPos.y = m_orbitRadius * sin(m_orbitPitch);
    m_cameraPos.z = m_orbitRadius * cos(m_orbitPitch) * cos(m_orbitYaw);

    // m_cameraFront = glm::normalize(glm::vec3(0.0f)-m_cameraPos);
    // m_cameraRight = glm::normalize(glm::cross(m_cameraPos, m_cameraUp));
    // // m_cameraUp    = glm::normalize(glm::cross(m_cameraPos, m_cameraRight));

    m_cameraFront = glm::normalize(glm::vec3(0.0f) - m_cameraPos);

    // Use world up to calculate right vector
    m_cameraRight = glm::normalize(glm::cross(m_worldUp, m_cameraFront));

    // Calculate camera up from right and front
    m_cameraUp = glm::normalize(glm::cross(m_cameraFront, m_cameraRight));

}

void Camera::updateOrbitAngles(float yawDelta, float pitchDelta)
{
    m_orbitYaw -= yawDelta;
    m_orbitPitch += pitchDelta;
    updateOrbit();
}

void Camera::resetPosition()
{
    m_cameraPos = m_originalCameraPos;
    m_cameraFront = glm::normalize(glm::vec3(0.0f) - m_cameraPos);
    setOrbit();
}

Camera::Camera(
    glm::vec3 cameraPos,
    float fov,
    float aspectRatio,
    float nearPlane,
    float farPlane,
    GLFWwindow* window
)
    : m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      m_cameraPos(cameraPos),
      m_originalCameraPos(cameraPos),
      m_cameraFront(glm::normalize(glm::vec3(0.0f) - cameraPos)),
      m_cameraRight(glm::cross(m_worldUp, m_cameraFront)),
      m_cameraUp(m_worldUp),
      m_FOV(fov),
      m_aspectRatio(aspectRatio),
      m_nearPlane(nearPlane),
      m_farPlane(farPlane),
      m_window(window),
      m_scrollSpeed(150.0f),
      m_mouseSensitivity(0.01),
      m_isDragging(false),
      m_lastX(0.0f),
      m_lastY(0.0f)
{
}

void Camera::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    float scrollAmount = camera->m_scrollSpeed * camera->m_deltaTime;
    camera->setPosition(camera->getPosition() + camera->getFront() * static_cast<float>(yoffset) * scrollAmount);
}

void Camera::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
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

void Camera::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera || !camera->isDragging()) return;

    float xoffset = static_cast<float>(xpos - camera->m_lastX);
    float yoffset = static_cast<float>(ypos - camera->m_lastY);

    camera->setLastMousePos(xpos, ypos);
    camera->updateOrbitAngles(xoffset * camera->m_mouseSensitivity, yoffset * camera->m_mouseSensitivity);
}
