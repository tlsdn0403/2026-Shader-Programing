// 편집기 UI: 패널과 마우스/키보드 입력을 관리하고 Settings를 수정합니다.
#pragma once
#include <filesystem>
#include <string>
#include <imgui.h>
#include "SceneSettings.h"
#include "LightEditor.h"
struct GLFWwindow;
class Renderer;

// Owns ImGui lifecycle and editor panels. Scene data remains owned by Application.
class EditorUI {
public:
    EditorUI() = default;
    EditorUI(const EditorUI&) = delete;
    EditorUI& operator=(const EditorUI&) = delete;
    ~EditorUI();
    void Initialize(GLFWwindow* window);
    void Shutdown();
    void BeginFrame();
    void Draw(Renderer& renderer, Settings& settings, float& elapsed,
              const std::filesystem::path& outputDirectory);
    void EndFrame(int width, int height);
    ImVec2 EffectCenter(int index) const { return effectCenters[index]; }
    ImVec2 PauseCenter() const { return pauseCenter; }
    ImVec2 ResetCenter() const { return resetCenter; }
    ImVec2 AddCubeCenter() const { return addCubeCenter; }
    ImVec2 DuplicateCenter() const { return duplicateCenter; }
    ImVec2 DeleteCenter() const { return deleteCenter; }
    ImVec2 GiDemoCenter() const { return giDemoCenter; }
    ImVec2 LodDemoCenter() const { return lodDemoCenter; }
    ImVec2 StressTabCenter() const { return stressTabCenter; }
    ImVec2 GenerateCenter() const { return generateCenter; }
    ImVec2 ObjectTabCenter() const { return objectTabCenter; }
    ImVec2 PhongCenter() const { return lightEditor.PhongCenter(); }
    ImVec2 DirectLightCenter() const { return lightEditor.EnabledCenter(); }
    ImVec2 LightTabCenter() const { return lightTabCenter; }
    ImVec2 FrameLightCenter() const { return lightEditor.FrameCenter(); }
    ImVec2 LightAxisCenter(int axis) const { return lightEditor.AxisCenter(axis); }
private:
    void DrawResearch(Renderer& renderer, Settings& settings);
    void DrawStress(Renderer& renderer, Settings& settings);
    int stressCount=1000, stressMesh=0, stressColumns=32;
    float stressSpacing=3.0f;
    bool stressVolume=false, stressGeometryOnly=true;
    ImVec2 stressTabCenter{}, generateCenter{};
    ImVec2 objectTabCenter{}, lightTabCenter{};
    LightEditor lightEditor;
    bool lightTabOpen=false;
    void DrawObjects(Settings& settings);
    ImVec2 addCubeCenter{}, duplicateCenter{}, deleteCenter{};
    ImVec2 giDemoCenter{}, lodDemoCenter{};
    int addCount = 1;
    void DrawToolbar(Renderer& renderer, Settings& settings, float& elapsed, float width);
    void DrawLibrary(Settings& settings);
    void DrawInspector(Renderer& renderer, Settings& settings, const std::filesystem::path& outputDirectory);
    void DrawPreview(Renderer& renderer, Settings& settings, float elapsed);
    bool contextReady = false, glfwReady = false, openglReady = false;
    bool orbit = false;
    std::string captureStatus;
    ImVec2 effectCenters[5]{};
    ImVec2 pauseCenter{};
    ImVec2 resetCenter{};
};
