#include "Camera.hpp"


void Camera::move()
{
    // float cameraSpeed = 30.0f * m_deltaTime;

    // if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    //     m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * cameraSpeed;
    // if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    //     m_cameraPos += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * cameraSpeed;
    // if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    //     m_cameraPos -= glm::normalize(m_cameraFront) * cameraSpeed;
    // if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    //     m_cameraPos += glm::normalize(m_cameraFront) * cameraSpeed;

    // if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
    //     m_cameraPos += m_cameraUp * cameraSpeed;
    // if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    //     m_cameraPos -= m_cameraUp * cameraSpeed;
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

    m_cameraFront = glm::normalize(-m_cameraPos); // Always look at origin
    m_cameraRight = glm::normalize(glm::cross(m_cameraUp, m_cameraPos));
    // m_cameraUp    = glm::normalize(glm::cross(m_cameraPos, m_cameraRight));

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
    m_cameraFront = glm::normalize(glm::vec3(0.0f) - m_cameraPos); // Point back to origin
    setOrbit();
}

Camera::Camera(
    glm::vec3 cameraPos,
    glm::vec3 cameraFront,
    glm::vec3 cameraUp,
    float fov,
    float aspectRatio,
    float nearPlane,
    float farPlane,
    GLFWwindow* window
)
    : m_cameraPos(cameraPos),
      m_originalCameraPos(cameraPos),
    //   m_cameraFront(cameraFront),
      m_cameraFront(glm::normalize(glm::vec3(0.0f) - cameraPos)),
      m_cameraUp(cameraUp),
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

    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (action == GLFW_PRESS) {
        camera->setDragging(true);
        camera->setOrbit();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        camera->setLastMousePos(x, y);
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