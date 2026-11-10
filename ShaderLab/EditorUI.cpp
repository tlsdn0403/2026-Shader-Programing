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
    "Phong / Blinn-Phong lighting. Select the model in the Light tab.",
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
    if(!io.WantTextInput && !ImGui::IsAnyItemActive()) {
        if(ImGui::IsKeyPressed(ImGuiKey_W)) objectEditor.SetMode(ObjectEditor::Move);
        if(ImGui::IsKeyPressed(ImGuiKey_E)) objectEditor.SetMode(ObjectEditor::Rotate);
        if(ImGui::IsKeyPressed(ImGuiKey_R)) objectEditor.SetMode(ObjectEditor::Scale);
        if(ImGui::IsKeyPressed(ImGuiKey_Space)) s.animate=!s.animate;
        if(ImGui::IsKeyPressed(ImGuiKey_F5)) renderer.Reload();
        if(ImGui::IsKeyPressed(ImGuiKey_Home)) FrameCamera(s);
    }
    float W=io.DisplaySize.x,H=io.DisplaySize.y,left=245,right=340,top=70,bottom=38;
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
    resetCenter={ImGui::GetItemRectMin().x+60,ImGui::GetItemRectMin().y+16};
}

// 실험 선택 시 프리셋을 적용합니다. 버튼 위치는 UI 자동 입력 테스트에서도 사용합니다.
void EditorUI::DrawLibrary(Settings& s) {
    Label("EXPERIMENTS"); ImGui::TextDisabled("Select a material study"); ImGui::Spacing();
    for(int i=0;i<5;++i) {
        if(ImGui::Selectable(names[i],(s.Selected().effect==i || (i==0 && s.Selected().effect==5)),0,{0,43})) Preset(s,i);
        effectCenters[i]={ImGui::GetItemRectMin().x+80,ImGui::GetItemRectMin().y+20};
    }
    DrawObjects(s);
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("ABOUT THIS STUDY");
    ImGui::TextWrapped("%s",descriptions[s.Selected().effect==5?0:s.Selected().effect]);
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("CONTROLS");
    ImGui::TextWrapped("Click object  /  Select\nW / E / R  /  Move / Rotate / Scale\nAlt + drag XYZ  /  Duplicate\nEsc  /  Cancel move\nDrag empty space  /  Orbit\nMouse wheel  /  Zoom\nHome  /  Frame scene\nSpace  /  Play or pause\nF5  /  Reload shaders");
    ImGui::Spacing(); ImGui::TextDisabled("OpenGL 3.3 / GLSL");
}

// 재질과 조명 값을 직접 수정합니다. 현재 선택한 효과에 필요한 항목만 표시합니다.
void EditorUI::DrawInspector(Renderer& renderer, Settings& s, const std::filesystem::path& outputDirectory) {
    lightTabOpen=false;
    if(!ImGui::BeginTabBar("Inspector tabs")) return;
    if(ImGui::BeginTabItem("Research")) { DrawResearch(renderer,s); ImGui::EndTabItem(); }
    bool stressOpen=ImGui::BeginTabItem("Stress");
    stressTabCenter={ImGui::GetItemRectMin().x+20,ImGui::GetItemRectMin().y+12};
    if(stressOpen) { DrawStress(renderer,s); ImGui::EndTabItem(); }
    bool objectOpen=ImGui::BeginTabItem("Object",nullptr,selectObjectTab?ImGuiTabItemFlags_SetSelected:0);
    selectObjectTab=false;objectTabCenter={ImGui::GetItemRectMin().x+20,ImGui::GetItemRectMin().y+12};
    if(objectOpen) {
    Label("INSPECTOR"); ImGui::TextDisabled("Live material parameters"); ImGui::Spacing();
    ImGui::SetNextItemWidth(-1);
    if(ImGui::Combo("##Mesh",&s.Selected().mesh,"Sphere\0Cube\0Subdivided plane\0")) FrameCamera(s);
    ImGui::Text("Selected: Object %d", s.Selected().id);
    ImGui::TextDisabled("Position X / Y / Z");
    ImGui::SetNextItemWidth(-1);
    ImGui::DragFloat3("##Position",s.Selected().position,0.05f,-30,30,"%.2f");
    ImGui::TextDisabled("Rotation X / Y / Z (degrees)");
    ImGui::SetNextItemWidth(-1);ImGui::DragFloat3("##Rotation",s.Selected().rotation,0.5f,0,0,"%.1f");
    ImGui::TextDisabled("Local scale X / Y / Z");
    ImGui::SetNextItemWidth(-1);ImGui::DragFloat3("##AxisScale",s.Selected().axisScale,0.01f,0.01f,100.f,"%.2f",ImGuiSliderFlags_AlwaysClamp);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##Scale",&s.Selected().scale,0.1f,3.0f,"Scale %.2f");
    if(ImGui::Button("Frame scene",{-1,30})) FrameCamera(s);
    if(s.Selected().effect==3) {ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Dissolve",&s.Selected().dissolve,0,1,"Dissolve  %.2f");}
    if(s.Selected().effect==4) {ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Wave",&s.Selected().wave,0,0.45f,"Wave height  %.2f");}
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("VIEWPORT");
    ImGui::Checkbox("Wireframe",&s.wireframe); ImGui::Checkbox("Reference floor",&s.floor);
    ImGui::TextDisabled("%d object triangles",renderer.Triangles(s.Selected().mesh));
    ImGui::Spacing();
    if(ImGui::Button("Save preview (.png)",{-1,34})) {
        auto path=outputDirectory/"preview.png";
        captureStatus=renderer.Texture() && renderer.Capture(path.string())?"Saved: "+path.string():"Capture failed";
    }
    if(!captureStatus.empty()) ImGui::TextWrapped("%s",captureStatus.c_str());
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); Label("SHADER STATUS");
    ImGui::Text("%s.vs",Renderer::ShaderName(s.Selected().effect));
    ImGui::Text("%s.fs",Renderer::ShaderName(s.Selected().effect));
    ImGui::TextWrapped("%s",renderer.Status().c_str());
    ImGui::EndTabItem();
    }
    bool textureOpen=ImGui::BeginTabItem("Texture");
    textureTabCenter={ImGui::GetItemRectMin().x+20,ImGui::GetItemRectMin().y+12};
    if(textureOpen) {
        auto& object=s.Selected();
        Label("BASE COLOR TEXTURE");
        ImGui::Text("Selected: Object %d",object.id);
        if(ImGui::Button("Choose image...",{-1,34})) {
            auto path=ChooseTextureFile();
            if(!path.empty()) renderer.ApplyTexture(object,path,textureStatus);
        }
        ImGui::TextDisabled("PNG / JPEG / BMP");
        if(!textureStatus.empty()) ImGui::TextWrapped("%s",textureStatus.c_str());
        if(const auto* image=renderer.TextureInfo(object.texturePath)) {
            ImGui::TextWrapped("%s",std::filesystem::u8path(object.texturePath).filename().u8string().c_str());
            ImGui::Text("%u x %u",image->width,image->height);
            float preview=std::min(ImGui::GetContentRegionAvail().x,180.f*image->width/image->height);
            ImGui::Image((ImTextureID)image->id,{preview,preview*image->height/image->width});
            ImGui::Checkbox("Enable texture",&object.textureEnabled);
            ImGui::Checkbox("Tint with Base color",&object.textureTint);
            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat2("##TextureRepeat",object.textureRepeat,0.25f,8.f,"Repeat %.2f");
            if(ImGui::Button("Remove texture",{-1,30})) {object.texturePath.clear();textureStatus.clear();}
        } else ImGui::TextWrapped("Choose an image to apply it to the selected object.");
        ImGui::TextWrapped("Color texture only. Image alpha is ignored. GI bounce uses the object's Base color.");
        ImGui::EndTabItem();
    }
    bool lightOpen=ImGui::BeginTabItem("Light");
    lightTabCenter={ImGui::GetItemRectMin().x+20,ImGui::GetItemRectMin().y+12};
    if(lightOpen) {lightTabOpen=true;lightEditor.DrawPanel(s);ImGui::EndTabItem();}
    else lightEditor.Cancel();
    ImGui::EndTabBar();
}

// 실제 구현한 기능과 원본 기술의 차이를 UI에 명시합니다.
void EditorUI::DrawResearch(Renderer& renderer, Settings& s) {
    Label("RENDERING RESEARCH");
    ImGui::TextWrapped("Voxel GI + meshlet LOD studies. Not UE Lumen / Nanite.");
    if(ImGui::Button("Load GI study",{-1,30})) LoadLightingDemo(s);
    giDemoCenter={ImGui::GetItemRectMin().x+60,ImGui::GetItemRectMin().y+15};
    if(ImGui::Button("Load 64-object LOD study",{-1,30})) LoadGeometryDemo(s);
    lodDemoCenter={ImGui::GetItemRectMin().x+60,ImGui::GetItemRectMin().y+15};
    ImGui::TextDisabled("Load replaces the current scene");
    ImGui::Separator();Label("DYNAMIC INDIRECT LIGHT");
    ImGui::TextWrapped("Edit GI and shadows in the Light tab.");
    ImGui::TextWrapped("64 cubed voxels. Volume: X/Z -8..8, Y -6..10. Outside: no GI.");
    ImGui::TextWrapped("Dissolve / wave objects receive GI but are excluded as GI sources and occluders.");
    ImGui::Text("Occupied: %d | Excluded: %d",renderer.GI().Occupied(),renderer.GI().Skipped());
    ImGui::Text("Last voxel CPU+upload: %.1f ms",renderer.GI().UpdateMs());
    ImGui::Separator();Label("GEOMETRY LOD + CULLING");
    ImGui::Checkbox("Automatic mesh LOD",&s.clusterLOD);
    ImGui::Checkbox("Cluster frustum culling",&s.clusterCulling);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##LODError",&s.lodPixels,0.1f,12,"LOD error %.1f px");
    ImGui::TextWrapped("CPU meshlet culling; object-level LOD. Wave keeps full detail. No streaming or occlusion culling.");
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##ResearchView",&s.researchView,"Shaded\0Indirect light only\0Cluster colors\0LOD colors\0");
    const auto& stats=renderer.Statistics();
    ImGui::Text("Triangles: %u / %u",stats.submittedTriangles,stats.fullTriangles);
    ImGui::Text("Clusters culled: %u / %u",stats.culled,stats.clusters);
    ImGui::Text("Object draws: %u",stats.drawCalls);
    ImGui::Text("LOD objects: %u / %u / %u / %u",stats.lodObjects[0],stats.lodObjects[1],stats.lodObjects[2],stats.lodObjects[3]);
    ImGui::Text("Scene GPU (delayed): %.2f ms",renderer.GpuMs());
    ImGui::TextDisabled("Counts exclude reference floor");
}

// Renderer가 FBO에 그린 결과를 이미지로 표시하고, 미리보기 영역의 카메라 입력을 처리합니다.
void EditorUI::DrawPreview(Renderer& renderer, Settings& s, float elapsed) {
    auto& io = ImGui::GetIO();
    ImGui::Text("%zu objects | Object %d",s.objects.size(),s.Selected().id); ImGui::SameLine(); ImGui::TextDisabled(" /  Perspective");
    if(ImGui::SmallButton("Move [W]")) objectEditor.SetMode(ObjectEditor::Move);
    ImGui::SameLine();if(ImGui::SmallButton("Rotate [E]")) objectEditor.SetMode(ObjectEditor::Rotate);
    ImGui::SameLine();if(ImGui::SmallButton("Scale [R]")) objectEditor.SetMode(ObjectEditor::Scale);
    ImGui::SameLine();ImGui::TextDisabled("Click an object to select");
    ImVec2 available=ImGui::GetContentRegionAvail();
    renderer.Render(static_cast<int>(available.x*io.DisplayFramebufferScale.x),static_cast<int>(available.y*io.DisplayFramebufferScale.y),s,elapsed);
    ImGui::Image(static_cast<ImTextureID>(renderer.Texture()),available,{0,1},{1,0});
    bool hovered=ImGui::IsItemHovered();
    previewOrigin=ImGui::GetItemRectMin();previewSize=available;
    auto vp=s.camera.ViewProjection(available.x/available.y);auto& selected=s.Selected();
    float w=vp[3]*selected.position[0]+vp[7]*selected.position[1]+vp[11]*selected.position[2]+vp[15];
    objectScreenCenter={previewOrigin.x+(1+(vp[0]*selected.position[0]+vp[4]*selected.position[1]+vp[8]*selected.position[2]+vp[12])/w)*available.x/2,
                        previewOrigin.y+(1-(vp[1]*selected.position[0]+vp[5]*selected.position[1]+vp[9]*selected.position[2]+vp[13])/w)*available.y/2};
    bool handle=false;
    if(lightTabOpen) {objectEditor.Cancel();handle=lightEditor.DrawGizmo(s,ImGui::GetItemRectMin(),available,hovered);}
    else {lightEditor.Cancel();handle=objectEditor.DrawGizmo(s,ImGui::GetItemRectMin(),available,hovered);}
    if(hovered && !handle) s.camera.Zoom(io.MouseWheel);
    if(ImGui::IsItemHovered()&&ImGui::IsMouseClicked(ImGuiMouseButton_Left)) ImGui::SetWindowFocus();
    
    if(hovered && !handle && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        unsigned id=renderer.Pick((io.MousePos.x-previewOrigin.x)/available.x,(io.MousePos.y-previewOrigin.y)/available.y);
        bool found=false;
        for(std::size_t i=0;i<s.objects.size();++i) if(static_cast<unsigned>(s.objects[i].id)==id) {
            s.selected=i;objectEditor.Show(true);lightTabOpen=false;selectObjectTab=true;found=true;break;
        }
        if(!found) objectEditor.Show(false);
        orbit=!found;
    }
    if(handle || !ImGui::IsMouseDown(ImGuiMouseButton_Left)) orbit=false;
    if(orbit && ImGui::IsMouseDragging(0)) {s.camera.Orbit(io.MouseDelta.x,io.MouseDelta.y);}
}

// 객체를 추가·선택·복제·삭제합니다. 벡터 변경 전에 참조를 보관하지 않아 무효 참조를 피합니다.
void EditorUI::DrawObjects(Settings& s) {
    ImGui::Spacing(); ImGui::Separator(); Label("SCENE OBJECTS");
    ImGui::Text("%zu / %zu objects",s.objects.size(),Settings::MaxObjects);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##AddCount",&addCount,1,16,"Add count: %d");
    auto add=[&](int mesh) {
        for(int i=0;i<addCount;++i) if(!s.AddObject(mesh)) break;
        FrameCamera(s);
    };
    ImGui::BeginDisabled(s.objects.size()>=Settings::MaxObjects);
    if(ImGui::Button("+ Sphere",{98,30})) add(0);
    ImGui::SameLine();
    if(ImGui::Button("+ Cube",{98,30})) add(1);
    addCubeCenter={ImGui::GetItemRectMin().x+40,ImGui::GetItemRectMin().y+15};
    if(ImGui::Button("+ Plane",{98,30})) add(2);
    ImGui::SameLine();
    if(ImGui::Button("Duplicate",{98,30})) {s.DuplicateSelected();FrameCamera(s);}
    duplicateCenter={ImGui::GetItemRectMin().x+40,ImGui::GetItemRectMin().y+15};
    ImGui::EndDisabled();
    ImGui::BeginDisabled(s.objects.size()<=1);
    if(ImGui::Button("Delete selected",{-1,30})) {s.RemoveSelected();FrameCamera(s);}
    deleteCenter={ImGui::GetItemRectMin().x+60,ImGui::GetItemRectMin().y+15};
    ImGui::EndDisabled();
    if(s.objects.size()==1) ImGui::TextDisabled("Keep at least one object");
    ImGui::BeginChild("Object list",{0,115},ImGuiChildFlags_Borders);
    const char* meshNames[]={"Sphere","Cube","Plane"};
    // 화면에 보이는 목록 항목만 제출해 수천 개 객체의 UI 비용을 줄입니다.
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(s.objects.size()));
    while(clipper.Step()) for(int row=clipper.DisplayStart;row<clipper.DisplayEnd;++row) {
        std::size_t i=static_cast<std::size_t>(row);
        const auto& object=s.objects[i];
        ImGui::PushID(object.id);
        char label[80];
        snprintf(label,sizeof(label),"%s %d / %s",meshNames[object.mesh],object.id,Renderer::ShaderName(object.effect));
        if(ImGui::Selectable(label,s.selected==i)) {s.selected=i;objectEditor.Show(true);}
        ImGui::PopID();
    }
    ImGui::EndChild();
}

// 별도 GPU 메시 복제 없이 공유 메시를 여러 위치에 그리는 부하 비교 장면입니다.
void EditorUI::DrawStress(Renderer& renderer,Settings& s) {
    Label("STRESS TEST");
    ImGui::Text("Current objects: %zu",s.objects.size());
    ImGui::SetNextItemWidth(-1);ImGui::InputInt("##StressCount",&stressCount,100,1000);
    stressCount=std::clamp(stressCount,1,static_cast<int>(Settings::MaxObjects));
    ImGui::TextDisabled("Object count: 1 - 4096");
    for(int count:{100,1000,4096}) {
        ImGui::PushID(count);
        if(ImGui::Button(std::to_string(count).c_str())) stressCount=count;
        ImGui::PopID();ImGui::SameLine();
    }
    ImGui::NewLine();
    ImGui::SetNextItemWidth(-1);ImGui::Combo("##StressMesh",&stressMesh,"Sphere\0Cube\0Plane\0Sphere + Cube\0");
    ImGui::Checkbox("3D grid",&stressVolume);
    ImGui::SetNextItemWidth(-1);ImGui::SliderInt("##Columns",&stressColumns,1,64,"Columns %d");
    ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Spacing",&stressSpacing,2.5f,10.f,"Spacing %.1f");
    ImGui::Checkbox("Disable GI for geometry test",&stressGeometryOnly);
    ImGui::TextWrapped("Generate replaces the scene and copies the selected material. LOD/culling settings are preserved.");
    if(ImGui::Button("Generate objects",{-1,32})) {
        GenerateStressScene(s,stressCount,stressMesh,stressColumns,stressSpacing,stressVolume);
        if(stressGeometryOnly) {s.voxelGI=false;s.voxelShadows=false;if(s.researchView==1)s.researchView=0;}
    }
    generateCenter={ImGui::GetItemRectMin().x+60,ImGui::GetItemRectMin().y+16};
    if(ImGui::Button("Keep selected only",{-1,28})) {
        SceneObject keep=s.Selected();s.objects.assign(1,keep);s.selected=0;FrameCamera(s);
    }
    if(ImGui::Button("Frame all objects",{-1,28})) FrameCamera(s);
    ImGui::Separator();
    ImGui::Checkbox("Automatic mesh LOD",&s.clusterLOD);
    ImGui::Checkbox("Cluster frustum culling",&s.clusterCulling);
    ImGui::Checkbox("Wireframe",&s.wireframe);
    const auto& stats=renderer.Statistics();
    ImGui::Text("Triangles: %u / %u",stats.submittedTriangles,stats.fullTriangles);
    ImGui::Text("Draw calls: %u",stats.drawCalls);
    ImGui::Text("Scene GPU: %.2f ms",renderer.GpuMs());
    ImGui::Text("FPS: %.1f",ImGui::GetIO().Framerate);
    ImGui::TextWrapped("Shared meshes, individual draw calls. Not GPU instancing. GI volume remains fixed at 16 units.");
}
