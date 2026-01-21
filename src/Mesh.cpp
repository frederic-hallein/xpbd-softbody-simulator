#include <set>

#include "logger.hpp"
#include "Object.hpp"
#include "Mesh.hpp"


void Mesh::constructVertices(const aiMesh* mesh)
{
    m_vertexToPositionIndex.clear();
    m_vertexToPositionIndex.reserve(mesh->mNumVertices);

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;

        // Vertex positions
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        // Keep track of only unique vertex positions
        auto pit = std::find(m_positions.begin(), m_positions.end(), vertex.position);
        int posIdx;
        if (pit == m_positions.end())
        {
            posIdx = m_positions.size();
            m_positions.push_back(vertex.position);
        }
        else
        {
            posIdx = std::distance(m_positions.begin(), pit);
        }

        m_positionToVertexIndices[posIdx].push_back(i);
        m_vertexToPositionIndex.push_back(posIdx);

        // Vertex texture coordinates
        if(mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        // Vertex normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        m_vertices.push_back(vertex);
    }
}

void Mesh::constructIndices(const aiMesh* mesh)
{
    m_indices.clear();
    size_t totalIndices = 0;
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        totalIndices += mesh->mFaces[i].mNumIndices;
    }

    m_indices.reserve(totalIndices);
    for(size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            m_indices.push_back(face.mIndices[j]);
        }
    }
}

void Mesh::constructMouseDistanceConstraintVertices(const aiMesh* mesh)
{
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices == 3) {
            Triangle triangle;
            triangle.v1 = m_vertexToPositionIndex[face.mIndices[0]];
            triangle.v2 = m_vertexToPositionIndex[face.mIndices[1]];
            triangle.v3 = m_vertexToPositionIndex[face.mIndices[2]];
            mouseDistanceConstraints.triangles.push_back(triangle);
        }
    }
}

void Mesh::constructDistanceConstraintVertices(const aiMesh* mesh)
{
    struct UniqueEdge
    {
        unsigned int v1;
        unsigned int v2;
        bool operator==(const UniqueEdge& other) const
        {
            return (v1 == other.v1 && v2 == other.v2) || (v1 == other.v2 && v2 == other.v1);
        }
        bool operator<(const UniqueEdge& other) const
        {
            int a1 = std::min(v1, v2), a2 = std::max(v1, v2);
            int b1 = std::min(other.v1, other.v2), b2 = std::max(other.v1, other.v2);
            return std::tie(a1, a2) < std::tie(b1, b2);
        }
    };

    std::set<UniqueEdge> uniqueEdges;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices == 3)
        {
            unsigned int idx[3];
            for (int j = 0; j < 3; ++j)
            {
                idx[j] = m_vertexToPositionIndex[face.mIndices[j]];
            }
            uniqueEdges.insert(UniqueEdge{idx[0], idx[1]});
            uniqueEdges.insert(UniqueEdge{idx[1], idx[2]});
            uniqueEdges.insert(UniqueEdge{idx[2], idx[0]});
        }
    }

    for (const auto& e : uniqueEdges)
    {
        Edge edge;
        edge.v1 = e.v1;
        edge.v2 = e.v2;
        distanceConstraints.edges.push_back(edge);
    }
}

void Mesh::constructVolumeConstraintVertices(const aiMesh* mesh)
{
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices == 3) {
            Triangle triangle;
            triangle.v1 = m_vertexToPositionIndex[face.mIndices[0]];
            triangle.v2 = m_vertexToPositionIndex[face.mIndices[1]];
            triangle.v3 = m_vertexToPositionIndex[face.mIndices[2]];
            volumeConstraints.triangles.push_back(triangle);
        }
    }
}

void Mesh::constructEnvCollisionConstraintVertices()
{
    std::set<unsigned int> uniqueIndices;

    // Directly iterate through m_vertices
    for (size_t i = 0; i < m_vertices.size(); ++i)
    {
        const glm::vec3& pos = m_vertices[i].position;

        // Find in m_positions
        auto it = std::find(m_positions.begin(), m_positions.end(), pos);
        if (it != m_positions.end())
        {
            unsigned int idx = static_cast<unsigned int>(std::distance(m_positions.begin(), it));
            uniqueIndices.insert(idx);
        }
    }

    envCollisionConstraintVertices.insert(
        envCollisionConstraintVertices.end(),
        uniqueIndices.begin(),
        uniqueIndices.end()
    );
}

void Mesh::loadObjData(const std::string& filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        filePath,
        aiProcess_Triangulate  | aiProcess_FlipUVs | aiProcess_GenSmoothNormals
    );

    if (!scene || !scene->HasMeshes())
    {
        logger::error("ASSIMP: Failed to load mesh: {}", filePath);
        return;
    }

    const aiMesh* mesh = scene->mMeshes[0];

    // Construct m_vertices and m_indices
    constructVertices(mesh);
    constructIndices(mesh);

    // construct vertices used for specific constraints
    constructMouseDistanceConstraintVertices(mesh);
    constructDistanceConstraintVertices(mesh);
    constructVolumeConstraintVertices(mesh);
    constructEnvCollisionConstraintVertices();
}

void Mesh::setCandidateObjectMeshes(const std::vector<Object*>& objects)
{
    for (auto* obj : objects)
    {
        // Skip null objects
        if (!obj) continue;

        const Mesh& objMesh = obj->getMesh();
        if (&objMesh != this)
        {
            m_candidateObjectMeshes.push_back(&objMesh);
        }
    }
}

void Mesh::constructDistanceConstraints()
{
    for (const auto& edge : distanceConstraints.edges)
    {
        unsigned int v1 = edge.v1;
        unsigned int v2 = edge.v2;
        float d_0 = glm::distance(m_positions[v1], m_positions[v2]);

        distanceConstraints.C.push_back([=](const std::vector<glm::vec3>& x) -> float {
            return glm::distance(x[v1], x[v2]) - d_0;
        });

        distanceConstraints.gradC.push_back([=](const std::vector<glm::vec3>& x) -> std::vector<glm::vec3> {
            std::vector<glm::vec3> grad(x.size(), glm::vec3(0.0f));
            glm::vec3 n = (x[v1] - x[v2]) / glm::distance(x[v1], x[v2]);
            grad[v1] = n;
            grad[v2] = -n;
            return grad;
        });
    }
}

void Mesh::constructVolumeConstraints(float& k)
{
    float V_0 = 0.0f;
    float factor = 1.0f / 6.0f;
    for (const auto& triangle : volumeConstraints.triangles)
    {
        unsigned int v1 = triangle.v1;
        unsigned int v2 = triangle.v2;
        unsigned int v3 = triangle.v3;
        V_0 += factor * glm::dot(glm::cross(m_positions[v1], m_positions[v2]), m_positions[v3]);
    }

    volumeConstraints.C.push_back([this, factor, V_0, k_ref = std::ref(k)](const std::vector<glm::vec3>& x) -> float {
        float V = 0.0f;
        for (const auto& triangle : volumeConstraints.triangles)
        {
            unsigned int v1 = triangle.v1;
            unsigned int v2 = triangle.v2;
            unsigned int v3 = triangle.v3;
            V += factor * glm::dot(glm::cross(x[v1], x[v2]), x[v3]);
        }

        return V - k_ref.get() * V_0;
    });

    volumeConstraints.gradC.push_back([this, factor](const std::vector<glm::vec3>& x) -> std::vector<glm::vec3> {
        std::vector<glm::vec3> grad(x.size(), glm::vec3(0.0f));
        for (const auto& triangle : volumeConstraints.triangles)
        {
            unsigned int v1 = triangle.v1;
            unsigned int v2 = triangle.v2;
            unsigned int v3 = triangle.v3;
            grad[v1] += factor * glm::cross(x[v2], x[v3]);
            grad[v2] += factor * glm::cross(x[v3], x[v1]);
            grad[v3] += factor * glm::cross(x[v1], x[v2]);
        }
        return grad;
    });
}

// TODO : fixme
void Mesh::constructEnvCollisionConstraints()
{
    // for (size_t meshIdx = 0; meshIdx < m_candidateObjectMeshes.size(); ++meshIdx)
    // {
    //     const auto& cMesh = m_candidateObjectMeshes[meshIdx];
    //     if (!cMesh) continue;

    //     // Create a new EnvCollisionConstraints for this mesh
    //     EnvCollisionConstraints envCollisionConstraints;
    //     envCollisionConstraints.candidateMesh = cMesh;

    //     // Get vertices from the candidate mesh
    //     const auto& vertices = cMesh->getVertices();

    //     // Loop through source vertices
    //     for (const auto& v : envCollisionConstraintVertices)
    //     {
    //         // Only process every third vertex (first vertex of each triangle)
    //         for (size_t vIdx = 0; vIdx < vertices.size(); vIdx += 3)
    //         {
    //             // Store the index where this constraint will be added
    //             size_t constraintIdx = envCollisionConstraints.C.size();

    //             // Add to map of vertex to constraint indices
    //             envCollisionConstraints.vertexToConstraints[v].push_back(constraintIdx);

    //             // Create constraint function
    //             envCollisionConstraints.C.push_back(
    //                 [v, vIdx, cMesh=cMesh](const std::vector<glm::vec3>& x) -> float {
    //                     const auto& cVertex = cMesh->getVertices()[vIdx];
    //                     float dot = glm::dot(cVertex.normal, x[v] - cVertex.position);
    //                     return dot;
    //                 });

    //             // Create gradient function
    //             envCollisionConstraints.gradC.push_back(
    //                 [v, vIdx, cMesh=cMesh](const std::vector<glm::vec3>& x) -> std::vector<glm::vec3> {
    //                     const auto& cVertex = cMesh->getVertices()[vIdx];
    //                     return { cVertex.normal };
    //                 });

    //             // Store affected vertex
    //             envCollisionConstraints.vertices.push_back(v);
    //         }
    //     }

    //     // Only add if we have constraints
    //     if (!envCollisionConstraints.C.empty())
    //     {
    //         perEnvCollisionConstraints.push_back(envCollisionConstraints);
    //     }
    // }
}

void Mesh::initVerticesBuffer()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_DYNAMIC_DRAW);

    // Bind and set EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void Mesh::initNormalBuffers()
{
    m_normalLines.vertexCount = m_vertices.size();
    m_normalLines.faceCount = m_indices.size() / 3;
    size_t totalLineVertices = m_normalLines.vertexCount + m_normalLines.faceCount;

    glGenVertexArrays(1, &m_normalLines.VAO);
    glGenBuffers(1, &m_normalLines.VBO);

    glBindVertexArray(m_normalLines.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalLines.VBO);

    glBufferData(GL_ARRAY_BUFFER, totalLineVertices * 2 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Mesh::Mesh(const std::string& name, const std::string& meshPath)
    : m_name(name),
      m_meshPath(meshPath),
      m_vertexNormalLength(0.1f),
      m_faceNormalLength(0.5f)
{
    loadObjData(meshPath);
    initVerticesBuffer();
    initNormalBuffers();
}

std::vector<glm::vec3> Mesh::calculateFaceNormals()
{
    std::vector<glm::vec3> faceNormals;
    size_t numTriangles = m_indices.size() / 3;
    faceNormals.reserve(numTriangles);

    for (size_t i = 0; i + 2 < m_indices.size(); i += 3)
    {
        unsigned int idx0 = m_indices[i];
        unsigned int idx1 = m_indices[i + 1];
        unsigned int idx2 = m_indices[i + 2];

        const glm::vec3& p_0 = m_vertices[idx0].position;
        const glm::vec3& p_1 = m_vertices[idx1].position;
        const glm::vec3& p_2 = m_vertices[idx2].position;

        glm::vec3 faceNormal = glm::normalize(glm::cross(p_1 - p_0, p_2 - p_0));
        faceNormals.push_back(faceNormal);
    }

    return faceNormals;
}

void Mesh::update()
{
    // update m_vertices positions
    size_t n = m_positions.size();
    for (size_t i = 0; i < n; ++i)
    {
        const glm::vec3& updatedPosition = m_positions[i];
        const auto& duplicates = m_positionToVertexIndices[i];
        for (unsigned int idx : duplicates)
        {
            m_vertices[idx].position = updatedPosition;
        }
    }


    std::vector<glm::vec3> updatedFaceNormals = calculateFaceNormals();
    for (size_t i = 0, tri = 0; i + 2 < m_indices.size(); i += 3, ++tri)
    {
        unsigned int idx0 = m_indices[i];
        unsigned int idx1 = m_indices[i + 1];
        unsigned int idx2 = m_indices[i + 2];

        glm::vec3& faceNormal = updatedFaceNormals[tri];

        m_vertices[idx0].normal = faceNormal;
        m_vertices[idx1].normal = faceNormal;
        m_vertices[idx2].normal = faceNormal;
    }
}

void Mesh::draw()
{
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(Vertex), &m_vertices[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::drawVertexNormals()
{
    std::vector<glm::vec3> lineVertices;
    lineVertices.reserve(m_normalLines.vertexCount * 2);
    for (const auto& v : m_vertices)
    {
        lineVertices.push_back(v.position);
        lineVertices.push_back(v.position + v.normal * m_vertexNormalLength);
    }

    glBindVertexArray(m_normalLines.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalLines.VBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, lineVertices.size() * sizeof(glm::vec3), lineVertices.data());

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertices.size()));
    glBindVertexArray(0);
}

void Mesh::drawFaceNormals()
{
    std::vector<glm::vec3> lineVertices;
    lineVertices.reserve(m_normalLines.faceCount * 2);

    for (size_t i = 0; i + 2 < m_indices.size(); i += 3)
    {
        unsigned int idx0 = m_indices[i];
        unsigned int idx1 = m_indices[i + 1];
        unsigned int idx2 = m_indices[i + 2];

        glm::vec3 centroid = (m_vertices[idx0].position + m_vertices[idx1].position + m_vertices[idx2].position) / 3.0f;
        glm::vec3 normal = m_vertices[idx0].normal;

        lineVertices.push_back(centroid);
        lineVertices.push_back(centroid + normal * m_faceNormalLength);
    }

    glBindVertexArray(m_normalLines.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalLines.VBO);

    size_t vertexNormalsSize = m_normalLines.vertexCount * 2 * sizeof(glm::vec3);
    glBufferSubData(GL_ARRAY_BUFFER, vertexNormalsSize, lineVertices.size() * sizeof(glm::vec3), lineVertices.data());

    glDrawArrays(GL_LINES, static_cast<GLint>(m_normalLines.vertexCount * 2), static_cast<GLsizei>(lineVertices.size()));
    glBindVertexArray(0);
}

void Mesh::destroy()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);

    glDeleteVertexArrays(1, &m_normalLines.VAO);
    glDeleteBuffers(1, &m_normalLines.VBO);
}