// 전체 흐름: 초기화 → 입력 처리 → 장면/UI 렌더링 → 화면 교환 → 자원 정리.
#ifdef _WIN32
#include <windows.h>
#endif
#include "Application.h"
#include "SmokeTests.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {
// 작업 폴더와 무관하게 실행 파일 옆의 Shaders 폴더를 찾습니다.
std::filesystem::path ExecutableDirectory(const char* argv0) {
#ifdef _WIN32
    wchar_t path[32768]{};
    DWORD length=GetModuleFileNameW(nullptr,path,32768);
    if(length && length<32768) return std::filesystem::path(path).parent_path();
#endif
    return std::filesystem::absolute(argv0).parent_path();
}
}
// 종료 순서가 중요합니다. GPU 자원은 OpenGL 컨텍스트를 없애기 전에 해제합니다.
Application::~Application() {
    // GPU resources must be released while their owning context is still current.
    if (window) glfwMakeContextCurrent(window);
    editor.Shutdown();
    if (graphicsReady) renderer.Shutdown();
    if (window) glfwDestroyWindow(window);
    if (glfwReady) glfwTerminate();
}
// 창과 OpenGL 함수 로더를 준비한 다음 셰이더·메시를 생성합니다.
void Application::Initialize(bool hidden, const char* executable) {
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW " << code << ": " << message << "\n";
    });
    glfwReady = glfwInit() == GLFW_TRUE;
    if (!glfwReady) throw std::runtime_error("GLFW initialization failed");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);
    window = glfwCreateWindow(1440, 900, "Shader Lab | Rendering Portfolio", nullptr, nullptr);
    if (!window) throw std::runtime_error("Window creation failed");
    glfwSetWindowSizeLimits(window, 1000, 680, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) throw std::runtime_error("OpenGL loader initialization failed");
    graphicsReady = true;
    while (glGetError() != GL_NO_ERROR) {}
    outputDirectory = ExecutableDirectory(executable);
    if (!renderer.Initialize((outputDirectory / "Shaders").string()))
        throw std::runtime_error(renderer.Status());
    std::cout << "GPU: " << glGetString(GL_RENDERER) << "\n";
}
// 실행 인수에 따라 일반 편집기 또는 숨겨진 창을 사용하는 테스트 모드를 선택합니다.
int Application::Run(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "";
    const bool smoke = mode == "--smoke-test", uiSmoke = mode == "--ui-smoke-test";
    try {
        Initialize(smoke || uiSmoke || mode == "--research-test", argv[0]);
        if (mode == "--research-test") return RunResearchTest(renderer);
        if (smoke) return RunRendererSmokeTest(renderer);
        editor.Initialize(window);
        RunEditor(uiSmoke);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
// 한 프레임의 실행 순서를 관리합니다. 테스트 입력은 UI가 입력을 읽기 전에 전달합니다.
void Application::RunEditor(bool testMode) {
    UiSmokeTest test;
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float delta = static_cast<float>(now - last);
        last = now;
        // 정지 후 복귀했을 때 애니메이션 시간이 과도하게 뛰지 않도록 시간 간격을 제한합니다.
        if (settings.animate) elapsed += std::min(delta, 0.1f);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        // 최소화된 창에는 그리지 않고 잠시 이벤트를 기다립니다.
        if (width == 0 || height == 0) { glfwWaitEventsTimeout(0.1); continue; }
        editor.BeginFrame();
        if (testMode) test.Inject(window, editor);
        editor.Draw(renderer, settings, elapsed, outputDirectory);
        editor.EndFrame(width, height);
        if (testMode) test.Verify(window, settings, width, height);
        glfwSwapBuffers(window);
    }
}
