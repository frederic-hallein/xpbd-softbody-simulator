#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "logger.hpp"
#include "ImGuiWindow.hpp"

ImGuiWindow::ImGuiWindow(GLFWwindow* window, const char* glslVersion)
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

void ImGuiWindow::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWindow::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiWindow::close()
{
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

void DebugWindow::displayPerformance(int frameDuration)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    float fps = 1000.0f / static_cast<float>(frameDuration);
    ImGui::Text("Frame Duration: %.3f ms", static_cast<float>(frameDuration));
    ImGui::Text("FPS: %.1f", fps);

    m_fpsHistory.push_back(fps);
    if (m_fpsHistory.size() > 120) {
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

void DebugWindow::displayCamera(Camera* camera)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Camera");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    const glm::vec3& cameraPosition = camera->getPosition();
    ImGui::Text("Pos: x = %.2f, y = %.2f, z = %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);

    ImGui::Text("Reset Camera [C]");
    ImGui::SameLine();
    if (ImGui::Button("Reset##ResetCamera") || ImGui::IsKeyPressed(ImGuiKey_C))
        camera->resetPosition();

    ImGui::Separator();
}

void DebugWindow::displayExternalForces(Scene& scene)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "External Forces");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    glm::vec3& gravitationalAcceleration = scene.getGravitationalAcceleration();
    ImGui::Text("Gravity");
    ImGui::SameLine();
    ImGui::SliderFloat("##Gravity", &gravitationalAcceleration.y, -9.81f, 9.81f);
    ImGui::Separator();
}

void DebugWindow::displayXPBDParameters(Scene& scene)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "XPBD");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    int& pbdSubsteps = scene.getPBDSubsteps();
    ImGui::Text("Substeps");
    ImGui::SameLine();
    ImGui::SliderInt("##Substeps n", &pbdSubsteps, 1, 30);

    bool& enableDistanceConstraints = scene.enableDistanceConstraints();
    ImGui::Checkbox("Enable Distance Constraints", &enableDistanceConstraints);

    bool& enableVolumeConstraints = scene.enableVolumeConstraints();
    ImGui::Checkbox("Enable Volume Constraints", &enableVolumeConstraints);

    bool& enableEnvCollisionConstraints = scene.enableEnvCollisionConstraints();
    ImGui::Checkbox("Enable Env Collision Constraints", &enableEnvCollisionConstraints);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    float& alpha = scene.getAlpha();
    ImGui::Text("alpha");
    ImGui::SameLine();
    ImGui::SliderFloat("##alpha", &alpha, 0.001f, 0.05f);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    float& beta = scene.getBeta();
    ImGui::Text("beta");
    ImGui::SameLine();
    ImGui::SliderFloat("##beta", &beta, 1.0f, 10.0f);
    ImGui::Separator();

    // float& k = scene.getOverpressureFactor();
    // ImGui::Text("k");
    // ImGui::SameLine();
    // ImGui::SliderFloat("##k", &k, 0.001f, 10.0f);
    // ImGui::Separator();
}

void DebugWindow::displaySceneReset(Scene& scene)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Scene Objects:");
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImGui::Text("Reset Scene [R]");
    ImGui::SameLine();
    if (ImGui::Button("Reset##ResetScene") || ImGui::IsKeyPressed(ImGuiKey_R))
    {
        for (auto& obj : scene.getObjects())
            obj->resetVertexTransforms();
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

void DebugWindow::displayVertexTransforms(size_t objectIndex, Object* object)
{
    if (!ImGui::TreeNode(("Vertex Transforms##" + std::to_string(objectIndex)).c_str()))
        return;

    const auto& vertexTransforms = object->getVertexTransforms();
    ImGui::Separator();
    for (size_t j = 0; j < vertexTransforms.size(); ++j)
    {
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

void DebugWindow::displayPolygonMode(size_t objectIndex, Object* object)
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

void DebugWindow::displayObjectPanel(size_t objectIndex, Object* object)
{
    std::string title = object->getName() + " " + std::to_string(objectIndex);

    if (!ImGui::CollapsingHeader(title.c_str()))
        return;

    displayVertexTransforms(objectIndex, object);
    displayPolygonMode(objectIndex, object);
}

void DebugWindow::displaySceneObjects(Scene& scene)
{
    const auto& objects = scene.getObjects();
    std::unordered_map<std::string, int> objectCounts;

    for (size_t i = 0; i < objects.size(); ++i)
    {
        Object* object = objects[i].get();
        int count = ++objectCounts[object->getName()];

        std::string title = object->getName() + " " + std::to_string(count);

        if (!ImGui::CollapsingHeader(title.c_str()))
            continue;

        displayVertexTransforms(i, object);
        displayPolygonMode(i, object);
    }
}

void DebugWindow::update(
    int frameDuration,
    Scene& scene
)
{
    ImGui::Begin("Debug");

    displayPerformance(frameDuration);
    displayCamera(scene.getCamera());
    displayExternalForces(scene);
    displayXPBDParameters(scene);
    displaySceneReset(scene);
    displaySceneObjects(scene);

    ImGui::End();
}