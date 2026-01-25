#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "SceneManager.hpp"
#include "glm/fwd.hpp"
#include "logger.hpp"
#include "ImGuiWindow.hpp"

ImGuiWindow::ImGuiWindow(
    GLFWwindow* window,
    const char* glslVersion
)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glslVersion);
    ImGui::StyleColorsDark();

    logger::info("ImGuiWindow created successfully");
}

void ImGuiWindow::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWindow::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiWindow::close() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    logger::info("ImGuiWindow closed successfully");
}

DebugWindow::DebugWindow(
    GLFWwindow* window,
    const char* glslVersion
)
    : ImGuiWindow(window, glslVersion)
{
}

void DebugWindow::displaySceneSelector(
    SceneManager& sceneManager
)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Scene Selection");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    const auto& scenes = sceneManager.getAllScenes();
    const std::string& currentScene = sceneManager.getCurrentSceneName();

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("##SceneCombo", currentScene.c_str())) {
        for (const auto& [sceneName, scene] : scenes) {
            bool isSelected = (currentScene == sceneName);
            if (ImGui::Selectable(sceneName.c_str(), isSelected)) {
                sceneManager.switchScene(sceneName);
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();
    ImGui::Separator();
}

void DebugWindow::displayPerformance(
    int frameDuration
)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    float fps = 1000.0f / static_cast<float>(frameDuration);
    ImGui::Text("Frame Duration: %.3f ms", static_cast<float>(frameDuration));
    ImGui::Text("FPS: %.1f", fps);

    m_fpsHistory.push_back(fps);
    if (m_fpsHistory.size() > 120.0f) {
        m_fpsHistory.pop_front();
    }

    if (!m_fpsHistory.empty()) {
        std::vector<float> fpsPlot(m_fpsHistory.begin(), m_fpsHistory.end());
        ImVec2 plotSize(ImGui::GetContentRegionAvail().x, 100);

        ImVec2 plotPos = ImGui::GetCursorScreenPos();
        ImGui::PlotLines(
            "##fpsplot",
            fpsPlot.data(),
            fpsPlot.size(),
            0,
            nullptr,
            0.0f,
            60.0f,
            plotSize
        );

        float y60 = plotSize.y * (1.0f - 60.0f / 60.0f);
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(p0.x, p0.y + y60),
            ImVec2(p1.x, p0.y + y60),
            IM_COL32(255, 255, 0, 255),
            0.5f
        );
    }

    ImGui::Separator();
}

void DebugWindow::displayCamera(
    Camera* camera
)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Camera");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    if (ImGui::Button("Reset Camera (or press C)##ResetCamera") || ImGui::IsKeyPressed(ImGuiKey_C)) {
        camera->resetPosition();
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Text("Position:");

    const glm::vec3& cameraPosition = camera->getPosition();
    ImGui::Text("x = %.2f, y = %.2f, z = %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);

    auto spherical = camera->getSphericalPosition();
    ImGui::Text("r = %.2f, theta = %.2f, phi = %.2f",
        spherical.radius,
        glm::degrees(spherical.theta),
        glm::degrees(spherical.phi)
    );

    ImGui::Separator();
}

void DebugWindow::displayExternalForces(
    Scene& scene
)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "External Forces");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    glm::vec3& gravitationalAcceleration = scene.getGravitationalAcceleration();
    ImGui::Text("Gravity:");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 1);
    if (ImGui::Button("Reset")) {
        gravitationalAcceleration = glm::vec3(0.0f, -9.81f, 0.0f);
    }
    ImGui::SliderFloat("##Gravity", &gravitationalAcceleration.y, -50.0f, 50.0f);
    ImGui::PopItemWidth();
    ImGui::Separator();
}

void DebugWindow::displayXPBDParameters(
    Scene& scene
)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "XPBD");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    int& xpbdSubsteps = scene.getXPBDSubsteps();
    ImGui::Text("Substeps:");
    if (ImGui::Button("-")) {
        if (xpbdSubsteps > 1) xpbdSubsteps--;
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        if (xpbdSubsteps < 30) xpbdSubsteps++;
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 1);
    ImGui::SliderInt("##Substeps n", &xpbdSubsteps, 1, 30);
    ImGui::PopItemWidth();

    bool& enableDistanceConstraints = scene.enableDistanceConstraints();
    ImGui::Checkbox("Enable Distance Constraints", &enableDistanceConstraints);

    if (scene.getName() != "Cloth Sene") {
        bool& enableVolumeConstraints = scene.enableVolumeConstraints();
        ImGui::Checkbox("Enable Volume Constraints", &enableVolumeConstraints);
    }

    // bool& enableEnvCollisionConstraints = scene.enableEnvCollisionConstraints();
    // ImGui::Checkbox("Enable Collision Constraints", &enableEnvCollisionConstraints);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    float& alpha = scene.getAlpha();
    float alpha_min = 0.0f;
    float alpha_max = 1.0f;
    ImGui::Text("Compliance:");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 1);
    ImGui::SliderFloat("##Compliance", &alpha, alpha_min, alpha_max);
    ImGui::PopItemWidth();

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    float& beta = scene.getBeta();
    float beta_max = 10.0f;
    ImGui::Text("Damping:");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 1);
    ImGui::SliderFloat("##Damping", &beta, 0.0f, beta_max);
    ImGui::PopItemWidth();
    ImGui::Separator();

    // float& k = scene.getOverpressureFactor();
    // float k_max = 10.0f;
    // ImGui::SameLine();
    // if (ImGui::Button("Reset")) {
    //     k = 1.0f;
    // }
    // ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 1);
    // ImGui::SliderFloat("##k", &k, 0.0f, k_max);
    // ImGui::PopItemWidth();
    // ImGui::Separator();
}

void DebugWindow::displaySceneReset(
    Scene& scene
)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Scene Objects:");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    if (ImGui::Button("Reset Scene (or press R)##ResetScene") || ImGui::IsKeyPressed(ImGuiKey_R)) {
        for (auto& obj : scene.getObjects()) {
            obj->resetVertexTransforms();
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

void DebugWindow::displayVertexTransforms(
    size_t objectIndex,
    Object* object
)
{
    if (!ImGui::TreeNode(("Vertex Transforms##" + std::to_string(objectIndex)).c_str())) {
        return;
    }

    const auto& vertexTransforms = object->getVertexTransforms();
    ImGui::Separator();
    for (size_t j = 0; j < vertexTransforms.size(); ++j) {
        const glm::vec3& position = vertexTransforms[j].getPosition();
        const glm::vec3& velocity = vertexTransforms[j].getVelocity();
        const glm::vec3& acceleration = vertexTransforms[j].getAcceleration();
        ImGui::BulletText(
            "Vertex %zu:\nPos: (%.2f, %.2f, %.2f)\nVel: (%.2f, %.2f, %.2f)\nAcc: (%.2f, %.2f, %.2f)",
            j,
            position.x, position.y, position.z,
            velocity.x, velocity.y, velocity.z,
            acceleration.x, acceleration.y, acceleration.z
        );
    }
    ImGui::TreePop();
}

void DebugWindow::displayPolygonMode(
    size_t objectIndex,
    Object* object
)
{
    if (!ImGui::TreeNode(("Polygon Mode##" + std::to_string(objectIndex)).c_str()))
        return;

    GLenum currentMode = object->getPolygonMode();
    if (ImGui::RadioButton(("Fill##" + std::to_string(objectIndex)).c_str(), currentMode == GL_FILL))
        object->setPolygonMode(GL_FILL);

    if (ImGui::RadioButton(("Wireframe##" + std::to_string(objectIndex)).c_str(), currentMode == GL_LINE))
        object->setPolygonMode(GL_LINE);

    ImGui::TreePop();
}

void DebugWindow::displayNormalShaders(
    size_t objectIndex,
    Object* object
)
{
    if (!ImGui::TreeNode(("Normal Shaders##" + std::to_string(objectIndex)).c_str())) {
        return;
    }

    bool enableVertexNormals = object->getEnableVertexNormalShader();
    if (ImGui::Checkbox(("Vertex Normals##" + std::to_string(objectIndex)).c_str(), &enableVertexNormals)) {
        object->setEnableVertexNormalShader(enableVertexNormals);
    }

    bool enableFaceNormals = object->getEnableFaceNormalShader();
    if (ImGui::Checkbox(("Face Normals##" + std::to_string(objectIndex)).c_str(), &enableFaceNormals)) {
        object->setEnableFaceNormalShader(enableFaceNormals);
    }

    ImGui::TreePop();
}

void DebugWindow::displayObjectPanel(
    size_t objectIndex,
    Object* object
)
{
    std::string title = object->getName() + " " + std::to_string(objectIndex);
    if (!ImGui::CollapsingHeader(title.c_str())) {
        return;
    }

    displayVertexTransforms(objectIndex, object);
    displayPolygonMode(objectIndex, object);
}

void DebugWindow::displaySceneObjects(
    Scene& scene
)
{
    const auto& objects = scene.getObjects();
    std::unordered_map<std::string, int> objectCounts;

    for (size_t i = 0; i < objects.size(); ++i) {
        Object* object = objects[i].get();
        int count = ++objectCounts[object->getName()];

        std::string title = object->getName() + " " + std::to_string(count);

        if (!ImGui::CollapsingHeader(title.c_str())) {
            continue;
        }

        Mesh& mesh = object->getMesh();
        size_t vertexCount = mesh.getPositions().size();
        size_t edgeCount = mesh.distanceConstraints.edges.size();
        size_t triangleCount = mesh.volumeConstraints.triangles.size();

        ImGui::Text("Vertices: %zu", vertexCount);
        ImGui::Text("Edges: %zu", edgeCount);
        ImGui::Text("Triangles: %zu", triangleCount);
        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        float distanceEnergy = object->getDistanceConstraintEnergy();
        ImGui::Text("Distance Constraint Energy: %.2f J", distanceEnergy);

        float volumeEnergy = object->getVolumeConstraintEnergy();
        ImGui::Text("Volume Constraint Energy: %.2f J", volumeEnergy);
        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        displayVertexTransforms(i, object);
        displayPolygonMode(i, object);
        displayNormalShaders(i, object);
    }
}

void DebugWindow::update(
    int frameDuration,
    Scene& scene,
    SceneManager& sceneManager
)
{
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(300, 0),
        ImVec2(FLT_MAX, FLT_MAX)
    );
    ImGui::Begin("Debug");

    displaySceneSelector(sceneManager);
    displayPerformance(frameDuration);
    displayCamera(scene.getCamera());
    displayExternalForces(scene);
    displayXPBDParameters(scene);
    displaySceneReset(scene);
    displaySceneObjects(scene);

    ImGui::End();
}