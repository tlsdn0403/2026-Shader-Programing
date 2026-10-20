// 실제 GPU 렌더링과 UI 입력을 검증하는 테스트 진입점입니다.
#pragma once
#include "Camera.h"
#include <imgui.h>
struct GLFWwindow;
struct Settings;
class Renderer;
class EditorUI;
int RunRendererSmokeTest(Renderer& renderer);
int RunResearchTest(Renderer& renderer);
class UiSmokeTest {
public:
    void Inject(GLFWwindow* window, const EditorUI& editor);
    void Verify(GLFWwindow* window, const Settings& settings, int width, int height);
private:
    int testFrame = 0;
    Vec3 beforeLight{},beforeCamera{};
    ImVec2 axisMouse{};
};
