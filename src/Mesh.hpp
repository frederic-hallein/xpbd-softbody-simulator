#pragma once

#include <assimp/mesh.h>
#include <string>
#include <vector>
#include <functional>
#include <glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>

using Constraint = std::function<float(const std::vector<glm::vec3>&)>;
using ConstraintGradient = std::function<std::vector<glm::vec3>(const std::vector<glm::vec3>&)>;

class Object; // Forward declaration
class Mesh
{
public:
    Mesh() = default;
    Mesh(
        const std::string& name,
        const std::string& meshPath
    );

    const std::string getName()     const { return m_name; }
    const std::string getMeshPath() const { return m_meshPath; }

    void update();
    void draw();
    void drawVertexNormals();
    void drawFaceNormals();
    void destroy();

    void setCandidateObjectMeshes(const std::vector<Object*>& objects);

    void constructDistanceConstraints();
    void constructVolumeConstraints(float& k);
    void constructEnvCollisionConstraints();

public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct Edge
    {
        unsigned int v1;
        unsigned int v2;
    };

    struct Triangle
    {
        unsigned int v1;
        unsigned int v2;
        unsigned int v3;
    };

    std::vector<glm::vec3>& getPositions() { return m_positions; }
    const std::vector<Vertex>& getVertices() const { return m_vertices; }

    struct MouseDistanceConstraints
    {
        std::vector<Triangle> triangles;
        std::vector<Constraint> C;
        std::vector<ConstraintGradient> gradC;
    };
    MouseDistanceConstraints mouseDistanceConstraints;

    struct DistanceConstraints
    {
        std::vector<Edge> edges;
        std::vector<Constraint> C;
        std::vector<ConstraintGradient> gradC;
    };
    DistanceConstraints distanceConstraints;

    struct VolumeConstraints
    {
        std::vector<Triangle> triangles;
        std::vector<Constraint> C;
        std::vector<ConstraintGradient> gradC;
    };
    VolumeConstraints volumeConstraints;

    std::vector<unsigned int> envCollisionConstraintVertices;
    struct EnvCollisionConstraints
    {
        std::vector<unsigned int> vertices;
        const Mesh* candidateMesh;
        std::vector<Constraint> C;
        std::vector<ConstraintGradient> gradC;
        std::map<unsigned int, std::vector<size_t>> vertexToConstraints; // TODO : size_t ???
    };
    std::vector<EnvCollisionConstraints> perEnvCollisionConstraints;

private:
    void loadObjData(const std::string& meshPath);

    void initVerticesBuffer();
    void initNormalBuffers(GLuint& vao, GLuint& vbo, size_t numElements);

    void constructVertices(const aiMesh* mesh);
    void constructIndices(const aiMesh* mesh);

    std::vector<glm::vec3> calculateFaceNormals();

    void constructMouseDistanceConstraintVertices(const aiMesh* mesh);
    void constructDistanceConstraintVertices(const aiMesh* mesh);
    void constructVolumeConstraintVertices(const aiMesh* mesh);
    void constructEnvCollisionConstraintVertices();

private:
    std::string m_name;
    std::string m_meshPath;

    std::vector<glm::vec3> m_positions;
    std::unordered_map<unsigned int, std::vector<unsigned int>> m_positionToVertexIndices;
    std::vector<unsigned int> m_vertexToPositionIndex;

    GLuint m_VAO, m_VBO, m_EBO;
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    struct NormalLines
    {
        GLuint VAO = 0;
        GLuint VBO = 0;
        size_t vertexCount = 0;
        size_t faceCount = 0;
    };
    NormalLines m_normalLines;
    void initNormalBuffers();
    float m_vertexNormalLength;
    float m_faceNormalLength;

    std::vector<const Mesh*> m_candidateObjectMeshes;
};