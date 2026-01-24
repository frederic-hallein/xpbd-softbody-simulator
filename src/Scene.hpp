#pragma once

#include <memory>
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
#include "Light.hpp"
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
    glm::vec3 color;
    bool isStatic;
};

struct SceneConfig {
    std::string name;
    std::vector<ObjectConfig> objects;
};

class Scene
{
public:
    struct PickResult {
        Object* object = nullptr;
        Mesh::Triangle triangle;
        glm::vec3 intersection;
        bool hit = false;
    };

public:
    Scene(
        GLFWwindow* window,
        unsigned int screenWidth,
        unsigned int screenHeight,
        ShaderManager* shaderManager,
        MeshManager* meshManager,
        TextureManager* textureManager
    );
    ~Scene();

    void loadSceneConfig(const std::string& configPath);

    void update(float deltaTime);
    void render();
    void clear();

    const std::string& getName() { return m_name; }
    Camera* getCamera() { return m_camera.get(); }
    Light* getLight() { return m_light.get(); }
    const std::vector<std::unique_ptr<Object>>& getObjects() const { return m_objects; }

    PickResult pickObject(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDir
    );
    void createMouseConstraints(const PickResult& pick);
    void updateMouseConstraints(
        const glm::vec3& cameraPos,
        const glm::vec3& rayDir
    );
    void releaseMouseConstraints() { m_activeMouseConstraint.isActive = false; }
    void solveMouseConstraints(
        std::vector<glm::vec3>& x,
        const std::vector<glm::vec3>& posDiff,
        const std::vector<float>& M,
        float deltaTime_s
    );

    float computeConstraintEnergy(
        float alpha,
        const std::vector<std::function<float(const std::vector<glm::vec3>&)>>& constraintFunctions,
        const std::vector<glm::vec3>& x
    );

    bool& enableDistanceConstraints() { return m_enableDistanceConstraints; }
    void solveDistanceConstraints(
        std::vector<glm::vec3>& x,
        const std::vector<glm::vec3>& posDiff,
        const std::vector<float>& M,
        float alphaTilde,
        float gamma,
        const Mesh::DistanceConstraints& distanceConstraints
    );
    void computeDistanceConstraintEnergy(
        Object& object,
        const std::vector<glm::vec3>& x,
        float alpha,
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
    void computeVolumeConstraintEnergy(
        Object& object,
        const std::vector<glm::vec3>& x,
        float alpha,
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
    std::string m_name;
    GLFWwindow* m_window;
    unsigned int m_screenWidth;
    unsigned int m_screenHeight;

    ShaderManager* m_shaderManager;
    MeshManager* m_meshManager;
    TextureManager* m_textureManager;

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Light> m_light;

    std::vector<std::unique_ptr<Object>> m_objects;

    glm::vec3 m_gravitationalAcceleration;
    float m_groundLevel;

    int m_xpbdSubsteps;

    struct ActiveMouseConstraint {
        bool isActive = false;
        Object* object = nullptr;
        Mesh::Triangle triangle;
        glm::vec3 intersectionPoint;
        std::array<float, 3> initialDistances;
    };
    ActiveMouseConstraint m_activeMouseConstraint;

    bool m_enableDistanceConstraints;
    bool m_enableVolumeConstraints;
    bool m_enableEnvCollisionConstraints;

    float m_alpha;
    float m_beta;
    float m_k;

private:
    std::unique_ptr<Camera> createCamera();
    std::unique_ptr<Light> createLight();

    std::unique_ptr<Object> createObject(const ObjectConfig& config);
    SceneConfig parseSceneConfig(const YAML::Node& sceneYaml);

    std::optional<glm::vec3> rayIntersectsTriangle(
        const glm::vec3& ray_origin,
        const glm::vec3& ray_vector,
        const Mesh::Triangle& triangle,
        const std::vector<Transform>& vertexTransforms
    );

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
        float deltaTime,
        const glm::vec3& cameraPos,
        const glm::vec3& rayDir
    );

    void applyGroundCollision(Object& object);

    void updateObjectTransform(Object& object);
    void updateObjectPhysics(
        Object& object,
        float deltaTime,
        const glm::vec3& cameraPos,
        const glm::vec3& rayDir
    );
    void updateObjects(
        float deltaTime,
        const glm::vec3& cameraPos,
        const glm::vec3& rayDir
    );
};