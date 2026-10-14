// 효과 전환, 카메라, 오류 복구, 창 크기 변경을 자동 검증합니다. 일반 실행에서는 호출하지 않습니다.
#include "Renderer.h"
#include "SmokeTests.h"
#include "EditorUI.h"
#include "ImageCapture.h"
#include <GLFW/glfw3.h>
#include <set>
#include <iostream>
#include <stdexcept>

// 실제 GPU에서 각 효과가 다른 화면을 만드는지, 재컴파일 실패 뒤에도 정상 화면이 유지되는지 확인합니다.
int RunRendererSmokeTest(Renderer& renderer) {
    Settings s; std::set<unsigned long long> images;
    for(int i=0;i<5;++i) {
        Preset(s,i); renderer.Render(640,480,s,1.0f);
        images.insert(renderer.Fingerprint());
        if(glGetError()!=GL_NO_ERROR) { std::cerr<<"OpenGL error in effect "<<i<<"\n"; return 1; }
        auto original=renderer.Fingerprint();
        if(renderer.ReloadFrom("invalid shader","invalid shader",i)) return 1;
        renderer.Render(640,480,s,1.0f);
        if(original!=renderer.Fingerprint()) {std::cerr<<"Effect reload recovery failed: "<<i<<"\n";return 1;}
        const std::string validVertex = "#version 330 core\nvoid main() { gl_Position = vec4(0.0); }";
        if(renderer.ReloadFrom(validVertex,"invalid fragment shader",i)) return 1;
        renderer.Render(640,480,s,1.0f);
        if(original!=renderer.Fingerprint()) return 1;
    }
    if(images.size()!=5) {std::cerr<<"Effects did not produce distinct frames\n";return 1;}
    Preset(s,0); renderer.Render(320,240,s,0); auto before=renderer.Fingerprint();
    s.camera.Orbit(60.0f,20.0f);
    s.camera.Zoom(2.0f);
    renderer.Render(320,240,s,0);
    if(before==renderer.Fingerprint()) {std::cerr<<"Camera movement had no effect\n";return 1;}
    FrameCamera(s);
    renderer.Render(320,240,s,0);
    if(before!=renderer.Fingerprint()) {std::cerr<<"Camera reset did not restore the view\n";return 1;}
    if(renderer.ReloadFrom("invalid shader","invalid shader")) return 1;
    renderer.Render(320,240,s,0);
    if(before!=renderer.Fingerprint()) {std::cerr<<"Failed reload did not preserve the frame\n";return 1;}
    if(!renderer.Reload()) return 1;
    for(int mesh=0;mesh<3;++mesh) {
        s.Selected().mesh=mesh; s.wireframe=true; renderer.Render(400,300,s,0);
        if(glGetError()!=GL_NO_ERROR) return 1;
    }
    // 서로 다른 객체를 함께 그리고, 한 객체의 재질/이동/삭제가 독립적으로 반영되는지 검증합니다.
    Settings multiple;
    multiple.floor=false;
    renderer.Render(640,480,multiple,0);
    auto singleFrame=renderer.Fingerprint();
    if(!multiple.AddObject(1)) return 1;
    multiple.Selected().position[0]=1.8f;
    multiple.Selected().effect=2;
    renderer.Render(640,480,multiple,0);
    auto twoFrame=renderer.Fingerprint();
    if(twoFrame==singleFrame || multiple.objects[0].effect!=0) return 1;
    multiple.Selected().scale=0.5f;
    renderer.Render(640,480,multiple,0);
    if(renderer.Fingerprint()==twoFrame) return 1;
    if(!multiple.DuplicateSelected() || multiple.objects.size()!=3) return 1;
    if(multiple.Selected().effect!=2 || multiple.Selected().scale!=0.5f) return 1;
    if(multiple.objects[1].id==multiple.objects[2].id) return 1;
    if(!multiple.RemoveSelected() || !multiple.RemoveSelected()) return 1;
    renderer.Render(640,480,multiple,0);
    if(renderer.Fingerprint()!=singleFrame || multiple.RemoveSelected()) return 1;
    for(std::size_t i=1;i<Settings::MaxObjects;++i) if(!multiple.AddObject(static_cast<int>(i%3))) return 1;
    if(multiple.AddObject(0) || multiple.DuplicateSelected()) return 1;
    FrameCamera(multiple);
    renderer.Render(640,480,multiple,0);
    if(glGetError()!=GL_NO_ERROR) return 1;
    Preset(s,3); s.wireframe=false; renderer.Render(960,640,s,0);
    if(!renderer.Capture("smoke-preview.png")) return 1;
    std::cout<<"PASS: five distinct effects, three meshes, resize, wireframe, shader reload and failure recovery.\n";
    return 0;
}
// 프레임 번호별로 클릭을 누르고 떼는 이벤트와 창 크기 변경을 예약합니다.
void UiSmokeTest::Inject(GLFWwindow* window, const EditorUI& editor) {
    auto& io = ImGui::GetIO();
    // Feed input through ImGui's event queue, then verify the actual widgets below.
    io.AddFocusEvent(true);
    if(testFrame>=2 && testFrame<12) {
        int selected=(testFrame-2)/2;
        io.AddMousePosEvent(editor.EffectCenter(selected).x,editor.EffectCenter(selected).y);
        io.AddMouseButtonEvent(0,testFrame%2==0);
    }
    if(testFrame==12 || testFrame==13) {
        io.AddMousePosEvent(editor.PauseCenter().x,editor.PauseCenter().y);
        io.AddMouseButtonEvent(0,testFrame==12);
    }
    if(testFrame==24 || testFrame==25) {
        auto pos=editor.AddCubeCenter(); io.AddMousePosEvent(pos.x,pos.y);
        io.AddMouseButtonEvent(0,testFrame==24);
    }
    if(testFrame==28 || testFrame==29) {
        auto pos=editor.DuplicateCenter(); io.AddMousePosEvent(pos.x,pos.y);
        io.AddMouseButtonEvent(0,testFrame==28);
    }
    if(testFrame==32 || testFrame==33) {
        auto pos=editor.DeleteCenter(); io.AddMousePosEvent(pos.x,pos.y);
        io.AddMouseButtonEvent(0,testFrame==32);
    }
    if(testFrame==15) glfwSetWindowSize(window,1100,720);
    if(testFrame==18) glfwSetWindowSize(window,1440,900);
    if(testFrame==20 || testFrame==21) {
        io.AddMousePosEvent(editor.EffectCenter(0).x,editor.EffectCenter(0).y);
        io.AddMouseButtonEvent(0,testFrame==20);
    }
}
// 주입한 입력이 실제 상태에 반영됐는지 확인하고 마지막 프레임을 PNG로 남깁니다.
void UiSmokeTest::Verify(GLFWwindow* window, const Settings& s, int fw, int fh) {
    if(testFrame>=3 && testFrame<=11 && testFrame%2==1 && s.Selected().effect!=(testFrame-2)/2)
        throw std::runtime_error("UI effect selection failed");
    if(testFrame==14 && s.animate) throw std::runtime_error("UI pause button failed");
    if(testFrame==26 && (s.objects.size()!=2 || s.Selected().mesh!=1))
        throw std::runtime_error("Add cube button failed");
    if(testFrame==30 && s.objects.size()!=3) throw std::runtime_error("Duplicate button failed");
    if(testFrame==34 && s.objects.size()!=2) throw std::runtime_error("Delete button failed");
    if(testFrame==36) {
        if(s.Selected().effect!=0) throw std::runtime_error("UI selection after resize failed");
        if(!SaveFramebufferPng("ui-preview.png",fw,fh) || glGetError()!=GL_NO_ERROR) throw std::runtime_error("UI capture failed");
        glfwSetWindowShouldClose(window,GLFW_TRUE);
        std::cout<<"PASS: five UI material selections, pause button, and resized editor rendering.\n";
    }
    ++testFrame;
}
