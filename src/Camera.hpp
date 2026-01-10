#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

class Camera
{
public:
    Camera(
        glm::vec3 cameraPos,
        glm::vec3 cameraFront,
        glm::vec3 cameraUp,
        float FOV,
        float aspectRatio,
        float nearPlane,
        float farPlane,
        GLFWwindow* window
    );

    void move();

    const glm::vec3& getPosition() const { return m_cameraPos; }
    const glm::vec3& getFront()    const { return m_cameraFront; }
    const glm::vec3& getRight()    const { return m_cameraRight; }
    const glm::vec3& getUp()       const { return m_cameraUp; }
    float getFOV()                 const { return m_FOV; }
    float getAspectRatio()         const { return m_aspectRatio; }
    float getNearPlane()           const { return m_nearPlane; }
    float getFarPlane()            const { return m_farPlane; }

    bool isDragging()              const { return m_isDragging; }

    void setPosition(const glm::vec3& position) { m_cameraPos = position; }
    void setDeltaTime(float deltaTime) { m_deltaTime = deltaTime; }
    void setDragging(bool dragging) { m_isDragging = dragging; }
    void setLastMousePos(double x, double y) { m_lastX = x; m_lastY = y; }
    void updateOrbitAngles(float yawDelta, float pitchDelta);

    void setOrbit();
    void updateOrbit();
    void resetPosition();

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

private:
    glm::vec3 m_originalCameraPos;

    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraRight;
    glm::vec3 m_cameraUp;

    float m_FOV;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    float m_deltaTime;
    GLFWwindow* m_window;

    float m_scrollSpeed;
    float m_mouseSensitivity;

    bool m_isDragging;
    double m_lastX, m_lastY;

    float m_orbitYaw;
    float m_orbitPitch;
    float m_orbitRadius;
};