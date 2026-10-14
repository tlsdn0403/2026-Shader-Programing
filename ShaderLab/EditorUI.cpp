// ImGui로 툴바·실험 목록·Inspector·미리보기를 구성합니다. 실제 3D 그리기는 Renderer에 위임합니다.
#include "Renderer.h"
#include "EditorUI.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <stdexcept>

namespace {
const char* names[]={"01   Studio lighting","02   Toon shading","03   Fresnel rim","04   Dissolve","05   Wave surface"};
const char* descriptions[]={
    "Blinn-Phong lighting with adjustable specular response. A starting point for material studies.",
    "Four lighting bands and a hard specular highlight create an illustrated surface.",
    "View-dependent edge lighting. Rotate the camera to inspect the grazing angles.",
    "Layered procedural noise clips the surface and adds a hot emissive edge.",
    "Animated vertex displacement with corrected normals. Try the subdivided plane."};
// 패널 색상과 간격 등 편집기 전체에 적용할 스타일입니다.
void Style() {
    ImGui::StyleColorsDark(); auto& style=ImGui::GetStyle();
    style.WindowRounding=0; style.FrameRounding=5; style.GrabRounding=5;
    style.WindowPadding={18,16}; style.FramePadding={10,8}; style.ItemSpacing={9,10};
    auto* c=style.Colors;
    c[ImGuiCol_WindowBg]={0.055f,0.067f,0.086f,1}; c[ImGuiCol_Border]={0.14f,0.17f,0.21f,1};
    c[ImGuiCol_FrameBg]={0.10f,0.13f,0.17f,1}; c[ImGuiCol_Button]={0.10f,0.17f,0.21f,1};
    c[ImGuiCol_ButtonHovered]={0.13f,0.31f,0.37f,1}; c[ImGuiCol_ButtonActive]={0.14f,0.43f,0.48f,1};
    c[ImGuiCol_Header]=c[ImGuiCol_ButtonActive]; c[ImGuiCol_HeaderHovered]=c[ImGuiCol_ButtonHovered];
    c[ImGuiCol_SliderGrab]={0.29f,0.79f,0.78f,1}; c[ImGuiCol_SliderGrabActive]={0.49f,0.94f,0.90f,1};
    c[ImGuiCol_CheckMark]=c[ImGuiCol_SliderGrab]; c[ImGuiCol_Text]={0.85f,0.89f,0.93f,1};
}
void Label(const char* text) { ImGui::TextColored({0.36f,0.79f,0.79f,1},"%s",text); }
// 고정 배치 패널을 시작합니다. 호출한 쪽에서 ImGui::End()로 닫습니다.
void Panel(const char* name,float x,float y,float w,float h) {
    ImGui::SetNextWindowPos({x,y}); ImGui::SetNextWindowSize({w,h});
    ImGui::Begin(name,nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings);
}
}

EditorUI::~EditorUI() { Shutdown(); }
// ImGui 컨텍스트와 GLFW/OpenGL 연결을 순서대로 초기화합니다.
void EditorUI::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); contextReady = true;
    auto& io = ImGui::GetIO(); io.IniFilename = nullptr;
    if (std::filesystem::exists("C:/Windows/Fonts/segoeui.ttf"))
        io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 18);
    Style();
    glfwReady = ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (!glfwReady) throw std::runtime_error("ImGui GLFW initialization failed");
    openglReady = ImGui_ImplOpenGL3_Init("#version 330 core");
    if (!openglReady) throw std::runtime_error("ImGui OpenGL initialization failed");
}
// 초기화에 성공한 구성만 역순으로 정리하므로 중간 실패에도 안전하게 종료할 수 있습니다.
void EditorUI::Shutdown() {
    if (openglReady) ImGui_ImplOpenGL3_Shutdown();
    if (glfwReady) ImGui_ImplGlfw_Shutdown();
    if (contextReady) ImGui::DestroyContext();
    openglReady = glfwReady = contextReady = false;
}
// 입력과 렌더 백엔드의 프레임 준비입니다. 실제 ImGui 프레임은 Draw에서 시작합니다.
void EditorUI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}
// 단축키를 처리하고 각 패널을 배치합니다. UI 변경 사항은 Settings를 통해 렌더러로 전달됩니다.
void EditorUI::Draw(Renderer& renderer, Settings& s, float& elapsed,
                    const std::filesystem::path& outputDirectory) {
    ImGui::NewFrame();
    auto& io = ImGui::GetIO();
    if(!io.WantTextInput) {
        if(ImGui::IsKeyPressed(ImGuiKey_Space)) s.animate=!s.animate;
        if(ImGui::IsKeyPressed(ImGuiKey_R)) renderer.Reload();
        if(ImGui::IsKeyPressed(ImGuiKey_Home)) FrameCamera(s);
    }
    float W=io.DisplaySize.x,H=io.DisplaySize.y,left=245,right=285,top=70,bottom=38;
    Panel("Toolbar",0,0,W,top);
    DrawToolbar(renderer, s, elapsed, W);
    ImGui::End();
    Panel("Library",0,top,left,H-top-bottom);
    DrawLibrary(s);
    ImGui::End();
    Panel("Inspector",W-right,top,right,H-top-bottom);
    DrawInspector(renderer, s, outputDirectory);
    ImGui::End();
    Panel("Preview",left,top,W-left-right,H-top-bottom);
    DrawPreview(renderer, s, elapsed);
    ImGui::End();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(18,8));
    Panel("Status",0,H-bottom,W,bottom);
    ImGui::TextDisabled("%.0f FPS  |  %.2f ms / frame  |  %s  |  %s",io.Framerate,1000.0f/std::max(io.Framerate,1.0f),s.animate?"Playing":"Paused",glGetString(GL_RENDERER));
    ImGui::End();
    ImGui::PopStyleVar();
}
// 3D 미리보기 텍스처를 포함한 UI 전체를 창의 기본 프레임버퍼에 그립니다.
void EditorUI::EndFrame(int width, int height) {
    ImGui::Render();
    glViewport(0, 0, width, height);
    glClearColor(0.03f, 0.04f, 0.06f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// 재생·일시정지, 셰이더 재로딩, 장면 초기화 버튼입니다.
void EditorUI::DrawToolbar(Renderer& renderer, Settings& s, float& elapsed, float width) {
    Label("SHADER LAB"); ImGui::SameLine(165); ImGui::TextDisabled("/  REAL-TIME MATERIAL STUDIES");
    ImGui::SameLine(width-455);
    if(ImGui::Button(s.animate?"Pause":"Play",{78,32})) s.animate=!s.animate;
    pauseCenter={ImGui::GetItemRectMin().x+30,ImGui::GetItemRectMin().y+15};
    ImGui::SameLine(); if(ImGui::Button("Reload shaders",{145,32})) renderer.Reload();
    ImGui::SameLine(); if(ImGui::Button("Reset scene",{135,32})) {s=Settings{};elapsed=0;}
}

// 실험 선택 시 프리셋을 적용합니다. 버튼 위치는 UI 자동 입력 테스트에서도 사용합니다.
void EditorUI::DrawLibrary(Settings& s) {
    Label("EXPERIMENTS"); ImGui::TextDisabled("Select a material study"); ImGui::Spacing();
    for(int i=0;i<5;++i) {
        if(ImGui::Selectable(names[i],s.effect==i,0,{0,43})) Preset(s,i);
        effectCenters[i]={ImGui::GetItemRectMin().x+80,ImGui::GetItemRectMin().y+20};
    }
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("ABOUT THIS STUDY");
    ImGui::TextWrapped("%s",descriptions[s.effect]);
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("CONTROLS");
    ImGui::TextWrapped("Drag preview  /  Orbit\nMouse wheel  /  Zoom\nHome  /  Frame object\nSpace  /  Play or pause\nR  /  Reload shaders");
    ImGui::Spacing(); ImGui::TextDisabled("OpenGL 3.3 / GLSL");
}

// 재질과 조명 값을 직접 수정합니다. 현재 선택한 효과에 필요한 항목만 표시합니다.
void EditorUI::DrawInspector(Renderer& renderer, Settings& s, const std::filesystem::path& outputDirectory) {
    Label("INSPECTOR"); ImGui::TextDisabled("Live material parameters"); ImGui::Spacing();
    ImGui::SetNextItemWidth(-1);
    if(ImGui::Combo("##Mesh",&s.mesh,"Sphere\0Cube\0Subdivided plane\0")) FrameCamera(s);
    ImGui::ColorEdit3("Base color",s.color,ImGuiColorEditFlags_NoInputs);
    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##Roughness",&s.roughness,0.02f,1,"Specular width  %.2f");
    if(s.effect==2) {ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Rim",&s.rim,0.3f,8,"Rim power  %.2f");}
    if(s.effect==3) {ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Dissolve",&s.dissolve,0,1,"Dissolve  %.2f");}
    if(s.effect==4) {ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Wave",&s.wave,0,0.45f,"Wave height  %.2f");}
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("LIGHTING");
    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##Angle",&s.lightAngle,-3.14f,3.14f,"Light angle  %.2f");
    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##Power",&s.lightPower,0,8,"Intensity  %.2f");
    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##Ambient",&s.ambient,0,0.6f,"Ambient fill  %.2f");
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("VIEWPORT");
    ImGui::Checkbox("Wireframe",&s.wireframe); ImGui::Checkbox("Reference floor",&s.floor);
    ImGui::TextDisabled("%d object triangles",renderer.Triangles(s.mesh));
    ImGui::Spacing();
    if(ImGui::Button("Save preview (.png)",{-1,34})) {
        auto path=outputDirectory/"preview.png";
        captureStatus=renderer.Texture() && renderer.Capture(path.string())?"Saved: "+path.string():"Capture failed";
    }
    if(!captureStatus.empty()) ImGui::TextWrapped("%s",captureStatus.c_str());
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("SHADER STATUS");
    ImGui::Text("%s.vs",Renderer::ShaderName(s.effect));
    ImGui::Text("%s.fs",Renderer::ShaderName(s.effect));
    ImGui::TextWrapped("%s",renderer.Status().c_str());
}

// Renderer가 FBO에 그린 결과를 이미지로 표시하고, 미리보기 영역의 카메라 입력을 처리합니다.
void EditorUI::DrawPreview(Renderer& renderer, Settings& s, float elapsed) {
    auto& io = ImGui::GetIO();
    ImGui::Text("%s",names[s.effect]+5); ImGui::SameLine(); ImGui::TextDisabled(" /  Perspective");
    ImVec2 available=ImGui::GetContentRegionAvail();
    renderer.Render(static_cast<int>(available.x*io.DisplayFramebufferScale.x),static_cast<int>(available.y*io.DisplayFramebufferScale.y),s,elapsed);
    ImGui::Image(static_cast<ImTextureID>(renderer.Texture()),available,{0,1},{1,0});
    if(ImGui::IsItemHovered()) s.camera.Zoom(io.MouseWheel);
    if(ImGui::IsItemHovered()&&ImGui::IsMouseClicked(ImGuiMouseButton_Left)) ImGui::SetWindowFocus();
    
    if(ImGui::IsItemHovered()&&ImGui::IsMouseClicked(ImGuiMouseButton_Left)) orbit=true;
    if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) orbit=false;
    if(orbit) {s.camera.Orbit(io.MouseDelta.x,io.MouseDelta.y);}
}
