#pragma once

#include <span>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderManager.hpp"
#include "MeshManager.hpp"
#include "TextureManager.hpp"
#include "Camera.hpp"
#include "Object.hpp"

struct ObjectConfig
{
    std::string name;
    glm::vec3 position;
    glm::vec3 rotationAxis;
    float rotationDeg;
    glm::vec3 scale;
    std::string shaderName;
    std::string meshName;
    std::string textureName;
    bool isStatic;
};

struct SceneConfig {
    std::string name;
    std::vector<ObjectConfig> objects;
};

class Scene
{
public:
    Scene(
        GLFWwindow* window,
        unsigned int screenWidth,
        unsigned int screenHeight,
        ShaderManager* shaderManager,
        MeshManager* meshManager,
        TextureManager* textureManager
    );

    void loadSceneConfig(const std::string& configPath);

    void update(float deltaTime);
    void render();
    void clear();

    const std::string& getName() { return m_name; }
    Camera* getCamera() { return m_camera.get(); }
    const std::vector<std::unique_ptr<Object>>& getObjects() const { return m_objects; }

    bool& enableDistanceConstraints() { return m_enableDistanceConstraints; }
    void solveDistanceConstraints(
        std::vector<glm::vec3>& x,
        const std::vector<glm::vec3>& posDiff,
        const std::vector<float>& M,
        float alphaTilde,
        float gamma,
        const Mesh::DistanceConstraints& distanceConstraints
    );

    bool& enableVolumeConstraints() { return m_enableVolumeConstraints; }
    void solveVolumeConstraints(
        std::vector<glm::vec3>& x,
        const std::vector<glm::vec3>& posDiff,
        const std::vector<float>& M,
        float alphaTilde,
        float gamma,
        const Mesh::VolumeConstraints& volumeConstraints
    );

    bool& enableEnvCollisionConstraints() { return m_enableEnvCollisionConstraints; }
    void solveEnvCollisionConstraints(
        std::vector<glm::vec3>& x,
        const std::vector<glm::vec3>& posDiff,
        const std::vector<float>& M,
        float alphaTilde,
        float gamma,
        std::vector<Mesh::EnvCollisionConstraints> perEnvCollisionConstraints
    );

    glm::vec3& getGravitationalAcceleration() { return m_gravitationalAcceleration; }
    int& getXPBDSubsteps() { return m_xpbdSubsteps; }
    float& getAlpha() { return m_alpha; }
    float& getBeta()  { return m_beta;  }
    float& getOverpressureFactor() { return m_k; }

private:
    std::unique_ptr<Camera> createCamera(GLFWwindow* window, unsigned int screenWidth, unsigned int screenHeight);

    std::unique_ptr<Object> createObject(const ObjectConfig& config);
    SceneConfig parseSceneConfig(const YAML::Node& sceneYaml);

    void setupEnvCollisionConstraints();
    void applyGravity(
        Object& object,
        float deltaTime
    );
    float calculateDeltaLambda(
        float C_j,
        const std::vector<glm::vec3>& gradC_j,
        const std::vector<glm::vec3>& posDiff,
        std::span<const unsigned int> constraintVertices,
        const std::vector<float>& M,
        float alphaTilde,
        float gamma
    );
    // std::vector<glm::vec3> calculateDeltaX(
    //     float lambda,
    //     const std::vector<float>& M,
    //     std::vector<glm::vec3>& gradC_j,
    //     std::span<const unsigned int> constraintVertices
    // );
    void setDeltaX(
        std::vector<glm::vec3>& deltaX,
        float deltaLambda,
        const std::vector<float>& M,
        const std::vector<glm::vec3>& gradC_j,
        std::span<const unsigned int> constraintVertices
    );
    void updateConstraintPositions(
        std::vector<glm::vec3>& x,
        const std::vector<glm::vec3>& deltaX
    );
    void applyXPBD(
        Object& object,
        float deltaTime
    );

    void applyGroundCollision(Object& object);

    void updateObjectTransform(Object& object);
    void updateObjectPhysics(Object& object, float deltaTime);
    void updateObjects(float deltaTime);

private:
    std::string m_name;

    ShaderManager* m_shaderManager;
    MeshManager* m_meshManager;
    TextureManager* m_textureManager;

    std::unique_ptr<Camera> m_camera;

    std::vector<std::unique_ptr<Object>> m_objects;

    glm::vec3 m_gravitationalAcceleration;
    float m_groundLevel;

    int m_xpbdSubsteps;

    bool m_enableDistanceConstraints;
    bool m_enableVolumeConstraints;
    bool m_enableEnvCollisionConstraints;

    float m_alpha;
    float m_beta;
    float m_k;
};