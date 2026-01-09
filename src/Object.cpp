#include "logger.hpp"
#include "Object.hpp"

Object::Object(
    std::string name,
    Transform transform,
    float& k,
    Shader shader,
    Mesh mesh,
    std::optional<Texture> texture,
    bool isStatic
)
    : m_name(name),
      m_transform(std::move(transform)),
      m_shader(std::move(shader)),
      m_mesh(mesh),
      m_texture(texture),
      m_isStatic(isStatic),
      m_polygonMode(GL_FILL)
{

    std::vector<glm::vec3>& positions = m_mesh.getPositions();
    glm::mat3 rot = glm::mat3(m_transform.getModelMatrix());
    glm::vec3 trans = glm::vec3(m_transform.getModelMatrix()[3]);

    for (auto& pos : positions)
    {
        glm::vec3 newPos = rot * pos + trans;
        pos = newPos;

        Transform vertexTransform;
        vertexTransform.setPosition(pos);

        if (!m_isStatic)
        {
            vertexTransform.makeNotStatic();
        }

        m_initialVertexTransforms.push_back(vertexTransform);
        m_vertexTransforms.push_back(vertexTransform);
    }

    if (!m_isStatic)
    {
        m_polygonMode = GL_LINE;

        //create M array
        size_t n = m_vertexTransforms.size();
        m_M = std::vector<float>(n, 0.0f);
        for (size_t i = 0; i < n; ++i)
        {
            m_M[i] = m_vertexTransforms[i].getMass();
        }

        // create distance constraints
        m_mesh.constructDistanceConstraints();

        // create volume constraints
        m_mesh.constructVolumeConstraints(k);
    }

    logger::info("{} created", name);
}

Object::~Object()
{
    logger::info("{} destroyed", m_name);
}

void Object::update(float deltaTime)
{
    auto& positions = m_mesh.getPositions();
    auto& vertexTransforms = m_vertexTransforms;
    size_t n = positions.size();

    for (size_t i = 0; i < n; ++i)
    {
        positions[i] = vertexTransforms[i].getPosition();
    }

    m_mesh.update();
}

void Object::resetVertexTransforms()
{
    auto& positions = m_mesh.getPositions();
    auto& initialVertexTransforms = m_initialVertexTransforms;
    size_t n = positions.size();

    for (size_t i = 0; i < n; ++i)
    {
        positions[i] = initialVertexTransforms[i].getPosition();
        m_vertexTransforms[i].setPosition(initialVertexTransforms[i].getPosition());
        m_vertexTransforms[i].setVelocity(initialVertexTransforms[i].getVelocity());
    }

    m_mesh.update();
}

// TODO : refactoring
void Object::render()
{
    if (m_texture.has_value())
    {
        m_texture->bind();
    }

    glPolygonMode(GL_FRONT_AND_BACK, m_polygonMode);


    glm::vec3 lightDirection = glm::vec3(
        1.0f,
        0.5f,
        0.0
    );

    m_shader.useProgram();
    m_shader.setVec3("lightDir", lightDirection);

    int projectionLoc = glGetUniformLocation(m_shader.getID(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getProjectionMatrix()));

    int viewLoc = glGetUniformLocation(m_shader.getID(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getViewMatrix()));

    m_mesh.draw();


    // draw vertex normals
    s_vertexNormalShader.useProgram();

    projectionLoc = glGetUniformLocation(s_vertexNormalShader.getID(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getProjectionMatrix()));

    viewLoc = glGetUniformLocation(s_vertexNormalShader.getID(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getViewMatrix()));

    m_mesh.drawVertexNormals();


    // draw face normals
    s_faceNormalShader.useProgram();

    projectionLoc = glGetUniformLocation(s_faceNormalShader.getID(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getProjectionMatrix()));

    viewLoc = glGetUniformLocation(s_faceNormalShader.getID(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getViewMatrix()));

    m_mesh.drawFaceNormals();
}