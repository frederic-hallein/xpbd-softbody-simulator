#include "Transform.hpp"

Transform::Transform()
    : m_mass(1.0f),
      m_position(glm::vec3(0.0f)),
      m_velocity(glm::vec3(0.0f)),
      m_acceleration(glm::vec3(0.0f)),
      m_projection(glm::mat4(1.0f)),
      m_view(glm::mat4(1.0f)),
      m_model(glm::mat4(1.0f))
{
}

// defines how camera sees the scene
void Transform::setProjection(const Camera& camera)
{
    m_projection = glm::perspective(
        glm::radians(camera.getFOV()),
        camera.getAspectRatio(),
        camera.getNearPlane(),
        camera.getFarPlane()
    );
}

// defines positions of the object
void Transform::setModel(const glm::mat4& model)
{
    m_model = model;
}

// defines the position of the camera
void Transform::setView(const Camera& camera)
{
    m_view = glm::lookAt(
        camera.getPosition(),
        camera.getPosition() + camera.getFront(),
        camera.getUp()
    );
}
