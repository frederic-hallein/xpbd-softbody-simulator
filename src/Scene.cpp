#include <future>
#include <fstream>
#include <memory>
#include <yaml-cpp/yaml.h>

#include "logger.hpp"
#include "Scene.hpp"

Shader Object::s_vertexNormalShader;
Shader Object::s_faceNormalShader;

std::unique_ptr<Camera> Scene::createCamera() {
    float aspectRatio = static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight);
    return std::make_unique<Camera>(
        glm::vec3(0.0f, 5.0f, 20.0f),
        aspectRatio,
        m_window
    );
}

std::unique_ptr<Light> Scene::createLight() {
    return std::make_unique<Light>(glm::vec3(0.0f, 20.0f, 0.0f));
}

std::unique_ptr<Object> Scene::createObject(
    const ObjectConfig& config
)
{
    logger::info("  - Creating '{}' object...", config.name);
    Transform transform;
    transform.setProjection(*m_camera);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), config.position);
    model = glm::rotate(model, glm::radians(config.rotationDeg), config.rotationAxis);
    model = glm::scale(model, config.scale);

    transform.setModel(model);
    transform.setView(*m_camera);

    auto shaderOpt = m_shaderManager->getResource(config.shaderName);
    if (!shaderOpt) {
        logger::error("    - Shader '{}' not found for object '{}'", config.shaderName, config.name);
        return nullptr;
    }

    auto meshOpt = m_meshManager->getResource(config.meshName);
    if (!meshOpt) {
        logger::error("    - Mesh '{}' not found for object '{}'", config.meshName, config.name);
        return nullptr;
    }

    std::optional<std::reference_wrapper<Texture>> textureOpt;
    if (!config.textureName.empty()) {
        textureOpt = m_textureManager->getResource(config.textureName);
        if (!textureOpt) {
            logger::error("    - Texture '{}' not found for object '{}'", config.textureName, config.name);
            return nullptr;
        }
    } else {
        logger::warning("- Texture intentionally left empty for object '{}'", config.name);
    }

    if (textureOpt) {
        return std::make_unique<Object>(
            config.name,
            transform,
            m_k,
            shaderOpt->get(),
            meshOpt->get(),
            textureOpt->get(),
            config.isStatic,
            config.color
        );
    } else {
        return std::make_unique<Object>(
            config.name,
            transform,
            m_k,
            shaderOpt->get(),
            meshOpt->get(),
            std::nullopt,
            config.isStatic,
            config.color
        );
    }
}

void Scene::loadSceneConfig(
    const std::string& configPath
)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        logger::error("Failed to open scene config: {}", configPath);
        return;
    }

    YAML::Node sceneYaml = YAML::Load(configFile);
    configFile.close();

    SceneConfig sceneConfig = parseSceneConfig(sceneYaml);
    m_name = sceneConfig.name;

    auto vertexNormalShaderOpt = m_shaderManager->getResource("vertexNormal");
    auto faceNormalShaderOpt = m_shaderManager->getResource("faceNormal");

    if (!vertexNormalShaderOpt) {
        logger::error("Failed to load 'vertexNormal' shader for all objects");
    } else {
        Object::setVertexNormalShader(vertexNormalShaderOpt->get());
    }

    if (!faceNormalShaderOpt) {
        logger::error("Failed to load 'faceNormal' shader for all objects");
    } else {
        Object::setFaceNormalShader(faceNormalShaderOpt->get());
    }

    logger::info(" - Creating '{}' scene objects...", sceneConfig.name);
    for (const auto& config : sceneConfig.objects) {
        auto obj = createObject(config);
        if (!obj) {
            logger::error("Failed to create object: {}", config.name);
            continue;
        }

        m_objects.push_back(std::move(obj));

    }

    setupEnvCollisionConstraints(); // TODO : move
}

SceneConfig Scene::parseSceneConfig(
    const YAML::Node& sceneYaml
)
{
    SceneConfig config;
    config.name = sceneYaml["scene"]["name"].as<std::string>();

    const auto& objectsYaml = sceneYaml["scene"]["objects"];
    for (const auto& objYaml : objectsYaml) {
        ObjectConfig objConfig;
        objConfig.name = objYaml["name"].as<std::string>();
        objConfig.position = glm::vec3(
            objYaml["position"][0].as<float>(),
            objYaml["position"][1].as<float>(),
            objYaml["position"][2].as<float>()
        );
        objConfig.rotationAxis = glm::vec3(
            objYaml["rotationAxis"][0].as<float>(),
            objYaml["rotationAxis"][1].as<float>(),
            objYaml["rotationAxis"][2].as<float>()
        );
        objConfig.rotationDeg = objYaml["rotationDeg"].as<float>();
        objConfig.scale = glm::vec3(
            objYaml["scale"][0].as<float>(),
            objYaml["scale"][1].as<float>(),
            objYaml["scale"][2].as<float>()
        );
        objConfig.shaderName = objYaml["shader"].as<std::string>();
        objConfig.meshName = objYaml["mesh"].as<std::string>();
        objConfig.textureName = objYaml["texture"].as<std::string>();
        objConfig.color = glm::vec3(
            objYaml["color"][0].as<float>(),
            objYaml["color"][1].as<float>(),
            objYaml["color"][2].as<float>()
        );
        objConfig.isStatic = objYaml["isStatic"].as<bool>();

        config.objects.push_back(objConfig);
    }

    return config;
}

void Scene::setupEnvCollisionConstraints() {
    for (const auto& obj : m_objects) {
        if (obj->isStatic()) continue;

        std::vector<Object*> candidateObjects;
        candidateObjects.reserve(m_objects.size());
        for (const auto& objPtr : m_objects) {
            candidateObjects.push_back(objPtr.get());
        }

        Mesh& mesh = obj->getMesh();
        mesh.setCandidateObjectMeshes(candidateObjects);
        mesh.constructEnvCollisionConstraints();
    }
}

Scene::Scene(
    GLFWwindow* window,
    unsigned int screenWidth,
    unsigned int screenHeight,
    ShaderManager* shaderManager,
    MeshManager* meshManager,
    TextureManager* textureManager
)
    :   m_name(""),
        m_window(window),
        m_screenWidth(screenWidth),
        m_screenHeight(screenHeight),
        m_camera(createCamera()),
        m_light(createLight()),
        m_shaderManager(shaderManager),
        m_meshManager(meshManager),
        m_textureManager(textureManager),
        m_gravitationalAcceleration(0.0f, -9.81f, 0.0f),
        m_groundLevel(0.0f),
        m_barrierSize(30.0f),
        m_enableDistanceConstraints(true),
        m_enableVolumeConstraints(true),
        m_enableEnvCollisionConstraints(true),
        m_xpbdSubsteps(1),
        m_alpha(0.001f),
        m_beta(1.0f),
        m_k(1.0f)
{
}

Scene::~Scene()
{
}

void Scene::applyGravity(
    Object& object,
    float deltaTime
)
{
    for (auto& vertexTransform : object.getVertexTransforms()) {
        vertexTransform.setAcceleration(m_gravitationalAcceleration);
    }
}

float Scene::calculateDeltaLambda(
    float C_j,
    const std::vector<glm::vec3>& gradC_j,
    const std::vector<glm::vec3>& posDiff,
    std::span<const unsigned int> constraintVertices,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma
)
{
    float gradCMInverseGradCT = 0.0f;
    float gradCPosDiff = 0.0f;
    for (size_t i = 0; i < constraintVertices.size(); ++i)
    {
        unsigned int v = constraintVertices[i];
        float w = 1.0f / M[v];
        gradCMInverseGradCT += w * glm::dot(gradC_j[v], gradC_j[v]);
        gradCPosDiff += glm::dot(gradC_j[v], posDiff[v]);
    }

    return (-C_j - gamma * gradCPosDiff) / ((1 + gamma) * gradCMInverseGradCT + alphaTilde);
}

void Scene::setDeltaX(
    std::vector<glm::vec3>& deltaX,
    float deltaLambda,
    const std::vector<float>& M,
    const std::vector<glm::vec3>& gradC_j,
    std::span<const unsigned int> constraintVertices
)
{
    std::fill(deltaX.begin(), deltaX.end(), glm::vec3(0.0f));
    for (size_t i = 0; i < constraintVertices.size(); ++i) {
        unsigned int v = constraintVertices[i];
        float w = 1.0f / M[v];
        deltaX[v] = deltaLambda * w * gradC_j[v];
    }
}

void Scene::updateConstraintPositions(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& deltaX
)
{
    for (size_t k = 0; k < deltaX.size(); ++k) {
        x[k] += deltaX[k];
    }
}

/* Möller–Trumbore intersection algorithm (See Wikipedia) */
std::optional<glm::vec3> Scene::rayIntersectsTriangle(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const Mesh::Triangle& triangle,
    const std::vector<Transform>& vertexTransforms
)
{
    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    glm::vec3 v1 = vertexTransforms[triangle.v1].getPosition();
    glm::vec3 v2 = vertexTransforms[triangle.v2].getPosition();
    glm::vec3 v3 = vertexTransforms[triangle.v3].getPosition();

    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    glm::vec3 rayCrossEdge2 = glm::cross(rayDirection, edge2);
    float determinant = glm::dot(edge1, rayCrossEdge2);

    if (determinant > -epsilon && determinant < epsilon) {
        return {};
    }

    float inverseDeterminant = 1.0f / determinant;
    glm::vec3 rayOriginToV1 = rayOrigin - v1;
    float u = inverseDeterminant * glm::dot(rayOriginToV1, rayCrossEdge2);

    if ((u < 0.0f && glm::abs(u) > epsilon) || (u > 1.0f && glm::abs(u - 1.0f) > epsilon)) {
        return {};
    }

    glm::vec3 rayOriginToV1CrossEdge1 = glm::cross(rayOriginToV1, edge1);
    float v = inverseDeterminant * glm::dot(rayDirection, rayOriginToV1CrossEdge1);

    if ((v < 0.0f && glm::abs(v) > epsilon) || (u + v > 1.0f && glm::abs(u + v - 1.0f) > epsilon)) {
        return {};
    }

    float t = inverseDeterminant * glm::dot(edge2, rayOriginToV1CrossEdge1);
    if (t > epsilon) {
        return glm::vec3(rayOrigin + rayDirection * t);
    }

    return {};
}

Scene::PickResult Scene::pickObject(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir
)
{
    PickResult result;
    float closestDistance = std::numeric_limits<float>::max();

    for (const auto& objPtr : m_objects) {
        if (objPtr->isStatic()) continue;

        auto& vertexTransforms = objPtr->getVertexTransforms();
        const auto& triangles = objPtr->getMesh().mouseDistanceConstraints.triangles;
        for (const auto& triangle : triangles) {
            auto intersection = rayIntersectsTriangle(
                rayOrigin,
                rayDir,
                triangle,
                vertexTransforms
            );
            if (intersection) {
                float dist = glm::distance(rayOrigin, intersection.value());
                if (dist < closestDistance) {
                    closestDistance = dist;
                    result.object = objPtr.get();
                    result.triangle = triangle;
                    result.intersection = intersection.value();
                    result.hit = true;
                }
            }
        }
    }

    return result;
}

void Scene::createMouseConstraints(
    const PickResult& pick
)
{
    if (m_activeMouseConstraint.isActive) return;
    if (!pick.hit) return;

    auto& vertexTransforms = pick.object->getVertexTransforms();

    m_activeMouseConstraint.isActive = true;
    m_activeMouseConstraint.object = pick.object;
    m_activeMouseConstraint.triangle = pick.triangle;
    m_activeMouseConstraint.intersectionPoint = pick.intersection;

    m_activeMouseConstraint.initialDistances[0] = glm::distance(
        pick.intersection,
        vertexTransforms[pick.triangle.v1].getPosition()
    );
    m_activeMouseConstraint.initialDistances[1] = glm::distance(
        pick.intersection,
        vertexTransforms[pick.triangle.v2].getPosition()
    );
    m_activeMouseConstraint.initialDistances[2] = glm::distance(
        pick.intersection,
        vertexTransforms[pick.triangle.v3].getPosition()
    );

    logger::debug("Mouse constraint created for triangle ({}, {}, {})",
        pick.triangle.v1,
        pick.triangle.v2,
        pick.triangle.v3
    );
}

void Scene::updateMouseConstraints(
    const glm::vec3& cameraPos,
    const glm::vec3& rayDir
)
{
    if (!m_activeMouseConstraint.isActive) {
        return;
    }

    const glm::vec3& cameraFront = m_camera->getFront();
    const glm::vec3& planePoint = m_activeMouseConstraint.intersectionPoint;
    float denom = glm::dot(cameraFront, rayDir);
    if (glm::abs(denom) > 1e-6f) {
        float t = glm::dot(planePoint - cameraPos, cameraFront) / denom;
        if (t > 0.0f) {
            glm::vec3 rayPlaneIntersection = cameraPos + rayDir * t;
            m_activeMouseConstraint.intersectionPoint = rayPlaneIntersection;
        }
    }
}

void Scene::solveMouseConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float deltaTime_s
)
{
    const auto& constraint = m_activeMouseConstraint;
    const auto& triangle = constraint.triangle;
    glm::vec3 intersectionPos = constraint.intersectionPoint;
    std::vector<glm::vec3> deltaX(M.size(), glm::vec3(0.0f));

    constexpr float mouseAlpha = 0.0f;
    constexpr float mouseBeta = 1.0f;

    float mouseAlphaTilde = mouseAlpha / (deltaTime_s * deltaTime_s);
    float mouseBetaTilde = (deltaTime_s * deltaTime_s) * mouseBeta;
    float mouseGamma = (mouseAlphaTilde * mouseBetaTilde) / deltaTime_s;

    std::array<unsigned int, 3> triangleVertices = {
        triangle.v1,
        triangle.v2,
        triangle.v3
    };

    std::array<unsigned int, 1> vertex;
    for (int i = 0; i < 3; ++i) {
        unsigned int v = triangleVertices[i];

        float d_0 = constraint.initialDistances[i];
        float C_j = glm::distance(x[v], intersectionPos) - d_0;

        std::vector<glm::vec3> gradC_j(x.size(), glm::vec3(0.0f));
        glm::vec3 diff = x[v] - intersectionPos;
        float dist = glm::distance(x[v], intersectionPos);
        gradC_j[v] = glm::normalize(diff);

        vertex[0] = v;
        float deltaLambda = calculateDeltaLambda(
            C_j,
            gradC_j,
            posDiff,
            vertex,
            M,
            mouseAlphaTilde,
            mouseGamma
        );
        setDeltaX(deltaX, deltaLambda, M, gradC_j, vertex);
        updateConstraintPositions(x, deltaX);
    }
}

float Scene::computeConstraintEnergy(
    float alpha,
    const std::vector<Constraint>& constraints,
    const std::vector<glm::vec3>& x
)
{
    if (alpha == 0.0f) {
        return 0.0f;
    }

    float totalEnergy = 0.0f;
    for (const auto& constraint : constraints) {
        float C = constraint(x);
        totalEnergy += (0.5f / alpha) * (C * C);
    }

    return totalEnergy;
}

void Scene::solveDistanceConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma,
    const Mesh::DistanceConstraints& distanceConstraints
)
{
    std::vector<glm::vec3> deltaX(M.size(), glm::vec3(0.0f));
    for (size_t j = 0; j < distanceConstraints.edges.size(); ++j) {
        float C_j = distanceConstraints.C[j](x);
        std::vector<glm::vec3> gradC_j = distanceConstraints.gradC[j](x);
        const auto& edge = distanceConstraints.edges[j];
        const std::array<unsigned int, 2> constraintVertices = { edge.v1, edge.v2 };

        float deltaLambda = calculateDeltaLambda(
            C_j,
            gradC_j,
            posDiff,
            constraintVertices,
            M,
            alphaTilde,
            gamma
        );
        setDeltaX(deltaX, deltaLambda, M, gradC_j, constraintVertices);
        updateConstraintPositions(x, deltaX);
    }
}

void Scene::computeDistanceConstraintEnergy(
    Object& object,
    const std::vector<glm::vec3>& x,
    float alpha,
    const Mesh::DistanceConstraints& distanceConstraints
)
{
    float energy = computeConstraintEnergy(alpha, distanceConstraints.C, x);
    object.setDistanceConstraintEnergy(energy);
}

void Scene::solveVolumeConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma,
    const Mesh::VolumeConstraints& volumeConstraints
)
{
    float C_j = volumeConstraints.C[0](x);
    std::vector<glm::vec3> gradC_j = volumeConstraints.gradC[0](x);

    std::vector<unsigned int> constraintVertices;
    for (const auto& tri : volumeConstraints.triangles) {
        constraintVertices.push_back(tri.v1);
        constraintVertices.push_back(tri.v2);
        constraintVertices.push_back(tri.v3);
    }

    float deltaLambda = calculateDeltaLambda(
        C_j,
        gradC_j,
        posDiff,
        constraintVertices,
        M,
        alphaTilde,
        gamma
    );
    std::vector<glm::vec3> deltaX(M.size(), glm::vec3(0.0f));
    setDeltaX(deltaX, deltaLambda, M, gradC_j, constraintVertices);
    updateConstraintPositions(x, deltaX);
}

void Scene::computeVolumeConstraintEnergy(
    Object& object,
    const std::vector<glm::vec3>& x,
    float alpha,
    const Mesh::VolumeConstraints& volumeConstraints
)
{
    float energy = computeConstraintEnergy(alpha, volumeConstraints.C, x);
    object.setVolumeConstraintEnergy(energy);
}

// TODO : fixme
void Scene::solveEnvCollisionConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma,
    std::vector<Mesh::EnvCollisionConstraints> perEnvCollisionConstraints
)
{
    for (size_t setIdx = 0; setIdx < perEnvCollisionConstraints.size(); ++setIdx)
    {
        const auto& constraints = perEnvCollisionConstraints[setIdx];
        size_t verticesSize = constraints.vertices.size();
        size_t CSize = constraints.C.size();
        size_t gradCSize = constraints.gradC.size();

        if (verticesSize != gradCSize)
        {
            logger::error("EnvCollisionConstraints size mismatch in set {}", setIdx);
            continue;
        }

        for (const auto& [vertex, constraintIndices] : constraints.vertexToConstraints)
        {
            bool allNegative = true;
            float maxNegativeC = -std::numeric_limits<float>::max();
            size_t maxIdx = 0;
            for (size_t idx : constraintIndices)
            {
                float C_j = constraints.C[idx](x);
                if (C_j >= 0.0f)
                {
                    allNegative = false;
                }

                // Track the constraint with biggest negative value
                if (C_j < 0.0f && C_j > maxNegativeC)
                {
                    maxNegativeC = C_j;
                    maxIdx = idx;
                }
            }

            // If all constraints are negative, we have a collision with this vertex
            if (allNegative && !constraintIndices.empty())
            {
                float C_j = maxNegativeC;
                std::vector<glm::vec3> gradC_j = constraints.gradC[maxIdx](x);

                std::array<unsigned int, 1> constraintVertices = { vertex };

                float deltaLambda = calculateDeltaLambda(C_j, gradC_j, posDiff, constraintVertices, M, alphaTilde, gamma);

                std::vector<glm::vec3> deltaX(M.size(), glm::vec3(0.0f));
                setDeltaX(deltaX, deltaLambda, M, gradC_j, constraintVertices);
                updateConstraintPositions(x, deltaX);
            }
        }
    }
}

void Scene::applyXPBD(
    Object& object,
    float deltaTime,
    const glm::vec3& cameraPos,
    const glm::vec3& rayDir
)
{
    const auto& mesh = object.getMesh();
    const auto& distanceConstraints = mesh.distanceConstraints;
    const auto& volumeConstraints = mesh.volumeConstraints;
    const auto& perEnvCollisionConstraints = mesh.perEnvCollisionConstraints;

    auto& vertexTransforms = object.getVertexTransforms();
    const size_t numVerts = vertexTransforms.size();

    std::vector<float> M = object.getMass();
    std::vector<glm::vec3> x(numVerts);
    std::vector<glm::vec3> v(numVerts);
    std::vector<glm::vec3> p(numVerts);
    std::vector<glm::vec3> posDiff(numVerts);

    int subStep = 1;
    const int n = m_xpbdSubsteps;
    float deltaTime_s = deltaTime / static_cast<float>(n);

    float alphaTilde = m_alpha / (deltaTime_s * deltaTime_s);
    float betaTilde  = (deltaTime_s * deltaTime_s) * m_beta;
    float gamma      = (alphaTilde * betaTilde) / deltaTime_s;

    if (m_activeMouseConstraint.object == &object && m_activeMouseConstraint.isActive) {
        updateMouseConstraints(cameraPos, rayDir);
    }

    while (subStep < n + 1) {
        for (size_t i = 0; i < numVerts; ++i) {
            const Transform& vt = vertexTransforms[i];
            p[i] = vt.getPosition();
            v[i] = vt.getVelocity() + deltaTime_s * vt.getAcceleration();
            x[i] = p[i] + deltaTime_s * v[i];
            posDiff[i] = x[i] - p[i];
        }

        // Solve mouse constraints if active and
        if (m_activeMouseConstraint.object == &object && m_activeMouseConstraint.isActive) {
            solveMouseConstraints(
                x,
                posDiff,
                M,
                deltaTime_s
            );
        }

        // Distance constraints
        if (m_enableDistanceConstraints) {
            solveDistanceConstraints(
                x,
                posDiff,
                M,
                alphaTilde,
                gamma,
                distanceConstraints
            );

            computeDistanceConstraintEnergy(
                object,
                x,
                m_alpha,
                distanceConstraints
            );
        }

        // Volume constraints
        if (m_enableVolumeConstraints) {
            solveVolumeConstraints(
                x,
                posDiff,
                M,
                alphaTilde,
                gamma,
                volumeConstraints
            );

            computeVolumeConstraintEnergy(
                object,
                x,
                m_alpha,
                volumeConstraints
            );
        }

        // Environment Collision constraints
        if (m_enableEnvCollisionConstraints) {
            solveEnvCollisionConstraints(
                x,
                posDiff,
                M,
                alphaTilde,
                gamma,
                perEnvCollisionConstraints
            );
        }

        // Update positions and velocities
        for (size_t i = 0; i < numVerts; ++i) {
            Transform& vt = vertexTransforms[i];
            glm::vec3 newV = (x[i] - p[i]) / deltaTime_s;
            vt.setPosition(x[i]);
            vt.setVelocity(newV);
        }

        subStep++;
    }
}

void Scene::applyGroundCollision(Object& object) {
    auto& vertexTransforms = object.getVertexTransforms();
    for (auto& vertexTransform : vertexTransforms) {
        glm::vec3 pos = vertexTransform.getPosition();
        if (pos.y < m_groundLevel) {
            pos.y = m_groundLevel;
            vertexTransform.setPosition(pos);

            glm::vec3 vel = vertexTransform.getVelocity();
            if (vel.y < 0.0f) vel.y = 0.0f;
            vertexTransform.setVelocity(vel);
        }
    }
}

void Scene::applyInvisibleBarrierCollision(Object& object) {
    auto& vertexTransforms = object.getVertexTransforms();
    for (auto& vertexTransform : vertexTransforms) {
        glm::vec3 pos = vertexTransform.getPosition();
        glm::vec3 vel = vertexTransform.getVelocity();
        if (pos.x < -m_barrierSize) {
            pos.x = -m_barrierSize;
            if (vel.x < 0.0f) vel.x = 0.0f;
        } else if (pos.x > m_barrierSize) {
            pos.x = m_barrierSize;
            if (vel.x > 0.0f) vel.x = 0.0f;
        }

        if (pos.z < -m_barrierSize) {
            pos.z = -m_barrierSize;
            if (vel.z < 0.0f) vel.z = 0.0f;
        } else if (pos.z > m_barrierSize) {
            pos.z = m_barrierSize;
            if (vel.z > 0.0f) vel.z = 0.0f;
        }

        vertexTransform.setPosition(pos);
        vertexTransform.setVelocity(vel);
    }
}

void Scene::updateObjectPhysics(
    Object& object,
    float deltaTime,
    const glm::vec3& cameraPos,
    const glm::vec3& rayDir
)
{
    if (!object.isStatic()) {
        applyGravity(object, deltaTime);
        applyXPBD(object, deltaTime, cameraPos, rayDir);
        applyGroundCollision(object);
        applyInvisibleBarrierCollision(object);
    }
}

void Scene::updateObjectTransform(Object& object) {
    Transform& transform = object.getTransform();
    transform.setView(*m_camera);
}

void Scene::updateObjects(
    float deltaTime,
    const glm::vec3& cameraPos,
    const glm::vec3& rayDir
)
{
    std::vector<std::future<void>> futures;
    futures.reserve(m_objects.size());

    for (size_t i = 0; i < m_objects.size(); ++i) {
        futures.push_back(
            std::async(std::launch::async, [this, i, deltaTime, cameraPos, rayDir]() -> void {
                auto& object = m_objects[i];
                updateObjectTransform(*object);
                updateObjectPhysics(*object, deltaTime, cameraPos, rayDir);
                object->update(deltaTime);
            })
        );
    }

    for (auto& future : futures) {
        future.get();
    }
}

void Scene::update(float deltaTime) {
    m_camera->setDeltaTime(deltaTime);

    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);
    glm::vec3 rayDir = m_camera->getRayDirection(mouseX, mouseY, m_screenWidth, m_screenHeight);
    glm::vec3 cameraPos = m_camera->getPosition();

    updateObjects(deltaTime, cameraPos, rayDir);
}

void Scene::render() {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.820, 0.976, 0.973, 1.0f); // TODO : use skybox instead
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& object : m_objects) {
        object->render(m_light.get(), m_camera->getPosition(), m_barrierSize);
    }

}

void Scene::clear() {
    logger::info(" - Clearing '{}' scene...", m_name);
    m_textureManager->deleteAllResources();
    m_meshManager->deleteAllResources();
    m_shaderManager->deleteAllResources();
    m_objects.clear();

    logger::info(" - Cleared '{}' scene successfully", m_name);
}