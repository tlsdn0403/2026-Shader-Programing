// 편집기 UI: 패널과 마우스/키보드 입력을 관리하고 Settings를 수정합니다.
#pragma once
#include <filesystem>
#include <string>
#include <imgui.h>
#include "SceneSettings.h"
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
    ImVec2 AddCubeCenter() const { return addCubeCenter; }
    ImVec2 DuplicateCenter() const { return duplicateCenter; }
    ImVec2 DeleteCenter() const { return deleteCenter; }
private:
    void DrawObjects(Settings& settings);
    ImVec2 addCubeCenter{}, duplicateCenter{}, deleteCenter{};
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
};
