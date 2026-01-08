#pragma once

#include <string>
#include <glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Shader
{
public:
    Shader() = default;
    Shader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

    const std::string getName()         const { return m_name; }
    const std::string getVertexPath()   const { return m_vertexPath; }
    const std::string getFragmentPath() const { return m_fragmentPath; }
    const unsigned int getID()          const { return m_ID; }

    void useProgram();
    void destroy();

    void setUniform(const std::string& name, const glm::vec3& color);
    void setVec3(const std::string& name, const glm::vec3& value) const;

private:
    std::string m_name;
    std::string m_vertexPath;
    std::string m_fragmentPath;

    unsigned int m_ID;
};