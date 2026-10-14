// 실행 관리자: 창과 OpenGL 컨텍스트, 렌더러, UI, 장면 상태의 수명을 관리합니다.
#pragma once
#include "Renderer.h"
#include "EditorUI.h"
#include "SceneSettings.h"
struct GLFWwindow;

// Owns window/context, scene state, and frame scheduling.
class Application {
public:
    Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    ~Application();
    int Run(int argc, char** argv);
private:
    void Initialize(bool hidden, const char* executable);
    void RunEditor(bool testMode);
    GLFWwindow* window = nullptr;
    bool glfwReady = false, graphicsReady = false;
    Renderer renderer;
    EditorUI editor;
    Settings settings;
    std::filesystem::path outputDirectory;
    float elapsed = 0;
};
