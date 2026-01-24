#include "glm/fwd.hpp"
#include "logger.hpp"
#include "Object.hpp"

Object::Object(
    std::string name,
    Transform transform,
    float& k,
    Shader shader,
    Mesh mesh,
    std::optional<Texture> texture,
    bool isStatic,
    glm::vec3 color
)
    : m_name(name),
      m_transform(std::move(transform)),
      m_shader(std::move(shader)),
      m_mesh(mesh),
      m_texture(texture),
      m_color(color),
      m_isStatic(isStatic),
      m_polygonMode(GL_FILL),
      m_enablevertexNormalShader(false),
      m_enableFaceNormalShader(false)
{

    std::vector<glm::vec3>& positions = m_mesh.getPositions();
    glm::mat3 rot = glm::mat3(m_transform.getModelMatrix());
    glm::vec3 trans = glm::vec3(m_transform.getModelMatrix()[3]);

    for (auto& pos : positions) {
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

    if (!m_isStatic) {
        //create M array
        size_t n = m_vertexTransforms.size();
        m_M = std::vector<float>(n, 0.0f);
        for (size_t i = 0; i < n; ++i) {
            m_M[i] = m_vertexTransforms[i].getMass();
        }

        // create distance constraints
        m_mesh.constructDistanceConstraints();

        // create volume constraints
        m_mesh.constructVolumeConstraints(k);
    }

    logger::info("  - Created '{}' object successfully", name);
}

Object::~Object() {
    logger::info("  - Destroyed '{}' object successfully", m_name);
}

void Object::updateTransformWithCOM() {
    glm::vec3 centerOfMass = glm::vec3(0.0f);
    auto& positions = m_mesh.getPositions();
    for (const auto& pos : positions) {
        centerOfMass += pos;
    }
    centerOfMass /= static_cast<float>(positions.size());
    m_transform.setPosition(centerOfMass);
}

void Object::update(
    float deltaTime
)
{
    auto& positions = m_mesh.getPositions();
    auto& vertexTransforms = m_vertexTransforms;
    size_t n = positions.size();

    for (size_t i = 0; i < n; ++i) {
        positions[i] = vertexTransforms[i].getPosition();
    }

    m_mesh.update();
    updateTransformWithCOM();
}

void Object::resetVertexTransforms() {
    auto& positions = m_mesh.getPositions();
    auto& initialVertexTransforms = m_initialVertexTransforms;
    size_t n = positions.size();

    for (size_t i = 0; i < n; ++i) {
        positions[i] = initialVertexTransforms[i].getPosition();
        m_vertexTransforms[i].setPosition(initialVertexTransforms[i].getPosition());
        m_vertexTransforms[i].setVelocity(initialVertexTransforms[i].getVelocity());
    }

    m_mesh.update();
}

void Object::setProjectionViewUniforms(
    const Shader& shader
)
{
    int projectionLoc = glGetUniformLocation(shader.getID(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getProjectionMatrix()));

    int viewLoc = glGetUniformLocation(shader.getID(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_transform.getViewMatrix()));
}

void Object::render(
    Light* light,
    const glm::vec3& cameraPosition,
    float barrierSize
)
{
    glPolygonMode(GL_FRONT_AND_BACK, m_polygonMode);
    glLineWidth(3.0f);

    m_shader.useProgram();
    m_shader.setVec3("objectColor", m_color);
    m_shader.setVec3("lightColor", light->getColor());
    m_shader.setVec3("lightPos", light->getPosition());
    m_shader.setVec3("viewPos", cameraPosition);

    if (m_name == "Ground") {
        m_shader.setFloat("barrierSize", barrierSize);
    }

    if (m_texture) {
        m_texture->bind();
        m_shader.setInt("ourTexture", 0); // 0 for single texture
        m_shader.setInt("hasTexture", 1);
    } else {
        m_shader.setInt("hasTexture", 0);
    }

    setProjectionViewUniforms(m_shader);
    m_mesh.draw();

    glLineWidth(1.0f);
    if (m_enableFaceNormalShader) {
        s_faceNormalShader.useProgram();
        setProjectionViewUniforms(s_faceNormalShader);
        m_mesh.drawFaceNormals();
    }

    if (m_enablevertexNormalShader) {
        s_vertexNormalShader.useProgram();
        setProjectionViewUniforms(s_vertexNormalShader);
        m_mesh.drawVertexNormals();
    }
}