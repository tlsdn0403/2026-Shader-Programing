// 효과 전환, 카메라, 오류 복구, 창 크기 변경을 자동 검증합니다. 일반 실행에서는 호출하지 않습니다.
#include "Renderer.h"
#include "SmokeTests.h"
#include "EditorUI.h"
#include "ImageCapture.h"
#include <GLFW/glfw3.h>
#include <set>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <cmath>

// 기능을 끈 기준 화면과 비교하고, 캐시 무효화와 실제 제출 삼각형 수를 검증합니다.
int RunResearchTest(Renderer& renderer) {
    auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    // 광원 모델별 GPU 출력, 위치/색/방향/범위, GI 갱신을 실제 컨텍스트에서 검사합니다.
    Settings lightTest;lightTest.floor=false;lightTest.ambient=0;
    lightTest.light.innerAngle=3;lightTest.light.outerAngle=15; // 구 표면에 cone 경계가 걸리는 비교 장면
    std::set<unsigned long long> lightImages;
    const char* captures[]={"light-directional.png","light-point.png","light-spot.png"};
    for(int type=0;type<3;++type) {
        lightTest.light.type=type;renderer.Render(640,480,lightTest,0);
        lightImages.insert(renderer.Fingerprint());require(renderer.Capture(captures[type]),"Light capture failed");
        for(int effect=0;effect<6;++effect) {
            lightTest.Selected().effect=effect;renderer.Render(320,240,lightTest,0);
            require(glGetError()==GL_NO_ERROR,"Light/material GL error");
        }
        lightTest.Selected().effect=0;
    }
    require(lightImages.size()==3,"Light types did not produce different images");
    lightTest.light.type=LightSettings::Directional;renderer.Render(320,240,lightTest,0);
    auto directional=renderer.Fingerprint();
    lightTest.light.position[0]+=5;renderer.Render(320,240,lightTest,0);
    require(renderer.Fingerprint()==directional,"Directional lighting must ignore position");
    lightTest.light.type=LightSettings::Point;renderer.Render(320,240,lightTest,0);auto point=renderer.Fingerprint();
    lightTest.light.position[0]-=5;renderer.Render(320,240,lightTest,0);
    require(renderer.Fingerprint()!=point,"Point light movement had no effect");
    point=renderer.Fingerprint();lightTest.light.color[1]=0;renderer.Render(320,240,lightTest,0);
    require(renderer.Fingerprint()!=point,"Light color had no effect");
    lightTest.light.enabled=false;renderer.Render(320,240,lightTest,0);auto dark=renderer.Fingerprint();
    lightTest.light.enabled=true;lightTest.light.range=0.5f;renderer.Render(320,240,lightTest,0);
    require(renderer.Fingerprint()==dark,"Point light leaked beyond range");
    lightTest.light.range=20;lightTest.light.type=LightSettings::Spot;
    lightTest.light.direction[0]=0;lightTest.light.direction[1]=1;lightTest.light.direction[2]=0;
    renderer.Render(320,240,lightTest,0);
    require(renderer.Fingerprint()==dark,"Spot light leaked behind cone");
    lightTest.light.direction[1]=0;renderer.Render(320,240,lightTest,0);
    require(glGetError()==GL_NO_ERROR,"Zero direction fallback failed");
    lightTest.voxelGI=true;renderer.Render(320,240,lightTest,0);auto lightUpdates=renderer.GI().Updates();
    lightTest.light.direction[0]=0.2f;renderer.Render(320,240,lightTest,0);
    lightTest.light.range=15;renderer.Render(320,240,lightTest,0);
    lightTest.light.color[0]=0.5f;renderer.Render(320,240,lightTest,0);
    lightTest.light.type=LightSettings::Directional;renderer.Render(320,240,lightTest,0);
    require(renderer.GI().Updates()==lightUpdates+4,"Light edits did not invalidate GI");
    Settings s; LoadLightingDemo(s); s.voxelGI=false;
    renderer.Render(960,640,s,0); auto unlit=renderer.Fingerprint();
    require(renderer.Capture("research-gi-off.png"),"GI baseline capture failed");
    s.voxelGI=true; renderer.Render(960,640,s,0);
    require(renderer.Fingerprint()!=unlit,"GI did not change receiver lighting");
    require(renderer.GI().Occupied()>0,"Empty voxel volume");
    require(renderer.Capture("research-gi-on.png"),"GI capture failed");
    auto updates=renderer.GI().Updates();
    renderer.Render(960,640,s,0);
    require(renderer.GI().Updates()==updates,"Static GI cache rebuilt");
    s.objects[1].position[0]-=0.5f; renderer.Render(960,640,s,0);
    require(renderer.GI().Updates()==updates+1,"Moving a light source did not rebuild GI");
    s.objects[1].effect=3; renderer.Render(640,480,s,0);
    require(renderer.GI().Skipped()==1,"Dissolve caster exclusion failed");
    for(int effect=0;effect<6;++effect) {
        s.objects[0].effect=effect; renderer.Render(320,240,s,1);
        require(glGetError()==GL_NO_ERROR,"Research material GL error");
    }
    LoadGeometryDemo(s);s.researchView=0;s.clusterLOD=false;s.clusterCulling=false;
    renderer.Render(960,640,s,0); auto full=renderer.Statistics().submittedTriangles;
    s.clusterLOD=true;renderer.Render(960,640,s,0);
    auto lod=renderer.Statistics().submittedTriangles;
    auto noCulling=renderer.Fingerprint();
    require(lod>0 && lod<full,"LOD did not reduce triangles");
    s.clusterCulling=true;renderer.Render(960,640,s,0);
    auto culled=renderer.Statistics();
    require(renderer.Fingerprint()==noCulling,"Frustum culling changed visible pixels");
    require(culled.culled>0 && culled.submittedTriangles<lod,"Cluster culling did not reduce submissions");
    s.researchView=3;renderer.Render(960,640,s,0);
    require(renderer.Capture("research-lod.png"),"LOD capture failed");
    Settings wave;wave.objects[0].effect=4;wave.clusterLOD=true;
    renderer.Render(640,480,wave,1);
    require(renderer.Statistics().lodObjects[0]==1,"Wave must retain full detail");
    require(glGetError()==GL_NO_ERROR,"Research GL error");
    std::cout<<"PASS: GI toggle/cache/movement/materials, LOD "<<full<<" -> "<<lod
             <<", culling -> "<<culled.submittedTriangles<<" triangles.\n";
    return 0;
}

// 실제 GPU에서 각 효과가 다른 화면을 만드는지, 재컴파일 실패 뒤에도 정상 화면이 유지되는지 확인합니다.
int RunRendererSmokeTest(Renderer& renderer) {
    Settings s; std::set<unsigned long long> images;
    for(int i=0;i<6;++i) {
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
    if(images.size()!=6) {std::cerr<<"Effects did not produce distinct frames\n";return 1;}
    // 동일 장면에서 모델만 바꿔 출력 차이와 왕복 복원을 검사합니다.
    Settings lighting;lighting.floor=false;
    renderer.Render(640,480,lighting,0);auto blinn=renderer.Fingerprint();
    if(!renderer.Capture("lighting-blinn.png")) return 1;
    lighting.Selected().effect=5;renderer.Render(640,480,lighting,0);
    if(renderer.Fingerprint()==blinn || !renderer.Capture("lighting-phong.png")) return 1;
    lighting.Selected().effect=0;renderer.Render(640,480,lighting,0);
    if(renderer.Fingerprint()!=blinn) return 1;
    lighting.light.enabled=false;renderer.Render(640,480,lighting,0);auto off=renderer.Fingerprint();
    if(off==blinn) return 1;
    lighting.Selected().effect=5;renderer.Render(640,480,lighting,0);
    if(renderer.Fingerprint()!=off) return 1; // 직접광이 없으면 두 모델의 주변광 출력은 같습니다.
    lighting.light.intensity=0;lighting.light.enabled=true;renderer.Render(640,480,lighting,0);
    if(renderer.Fingerprint()!=off) return 1;
    lighting.voxelGI=true;lighting.light.intensity=3;renderer.Render(320,240,lighting,0);
    auto updates=renderer.GI().Updates();
    lighting.light.enabled=false;renderer.Render(320,240,lighting,0);
    if(renderer.GI().Updates()!=updates+1) return 1;
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
    // 최대 개수의 ID/배치/상한을 확인합니다. 대량 생성은 반복 AddObject 대신 O(N)으로 수행합니다.
    GenerateStressScene(multiple,5000,1,32,3,true);
    if(multiple.objects.size()!=Settings::MaxObjects) return 1;
    std::set<int> ids;
    std::set<std::tuple<float,float,float>> positions;
    for(const auto& o:multiple.objects) {
        ids.insert(o.id);positions.insert({o.position[0],o.position[1],o.position[2]});
    }
    if(ids.size()!=Settings::MaxObjects || positions.size()!=Settings::MaxObjects) return 1;
    if(multiple.AddObject(0) || multiple.DuplicateSelected()) return 1;
    FrameCamera(multiple);
    renderer.Render(640,480,multiple,0);
    if(renderer.Statistics().drawCalls!=Settings::MaxObjects) return 1;
    GenerateStressScene(multiple,1000,0,32,3,false);
    renderer.Render(640,480,multiple,0);
    if(renderer.Statistics().fullTriangles!=1000u*renderer.Triangles(0)) return 1;
    if(!renderer.Capture("stress-preview.png")) return 1;
    GenerateStressScene(multiple,0,3,0,0,false);
    if(multiple.objects.size()!=1 || !multiple.AddObject(1)) return 1;
    if(glGetError()!=GL_NO_ERROR) return 1;
    Preset(s,3); s.wireframe=false; renderer.Render(960,640,s,0);
    if(!renderer.Capture("smoke-preview.png")) return 1;
    std::cout<<"PASS: six shader pairs, Phong/Blinn comparison, light toggle, three meshes, resize, wireframe, reload recovery.\n";
    return 0;
}
// 프레임 번호별로 클릭을 누르고 떼는 이벤트와 창 크기 변경을 예약합니다.
void UiSmokeTest::Inject(GLFWwindow* window, const EditorUI& editor) {
    auto& io = ImGui::GetIO();
    if(testFrame==54) {auto pos=editor.LightTabCenter();io.AddMousePosEvent(pos.x,pos.y);}
    if(testFrame==55 || testFrame==56 || testFrame==60 || testFrame==61) {
        auto pos=testFrame<57?editor.LightTabCenter():editor.PhongCenter();
        io.AddMousePosEvent(pos.x,pos.y);
        io.AddMouseButtonEvent(0,testFrame==55 || testFrame==60);
    }
    if(testFrame==63 || testFrame==64) {
        auto p=editor.ResetCenter();io.AddMousePosEvent(p.x,p.y);io.AddMouseButtonEvent(0,testFrame==63);
    }
    if(testFrame==65) {auto p=editor.LightTabCenter();io.AddMousePosEvent(p.x,p.y);}
    if(testFrame==66 || testFrame==67 || testFrame==70 || testFrame==71 || testFrame==74 || testFrame==75) {
        auto p=testFrame<68?editor.LightTabCenter():testFrame<72?editor.DirectLightCenter():editor.FrameLightCenter();
        io.AddMousePosEvent(p.x,p.y);io.AddMouseButtonEvent(0,testFrame%2==0);
    }
    for(int axis=0;axis<3;++axis) {
        int start=80+axis*6;
        if(testFrame==start) {
            axisMouse=editor.LightAxisCenter(axis);
            if(axisMouse.x==0 && axisMouse.y==0) throw std::runtime_error("Gizmo axis not visible");
            io.AddMousePosEvent(axisMouse.x,axisMouse.y);io.AddMouseButtonEvent(0,true);
        }
        if(testFrame==start+1) io.AddMousePosEvent(axisMouse.x+20,axisMouse.y+20);
        if(testFrame==start+2) io.AddMouseButtonEvent(0,false);
    }
    if(testFrame==98 || testFrame==99) {
        auto p=editor.DirectLightCenter();io.AddMousePosEvent(p.x,p.y);io.AddMouseButtonEvent(0,testFrame==98);
    }
    if(testFrame==101) {
        axisMouse=editor.LightAxisCenter(0);io.AddMousePosEvent(axisMouse.x,axisMouse.y);io.AddMouseButtonEvent(0,true);
    }
    if(testFrame==102) io.AddMousePosEvent(axisMouse.x+30,axisMouse.y+20);
    if(testFrame==103) io.AddKeyEvent(ImGuiKey_Escape,true);
    if(testFrame==104) {io.AddKeyEvent(ImGuiKey_Escape,false);io.AddMouseButtonEvent(0,false);}
    // 탭은 overlap 허용 위젯이므로 먼저 hover한 다음 프레임에 클릭합니다.
    if(testFrame==45) {auto pos=editor.StressTabCenter();io.AddMousePosEvent(pos.x,pos.y);}
    if(testFrame==46 || testFrame==47 || testFrame==50 || testFrame==51) {
        auto pos=testFrame<48?editor.StressTabCenter():editor.GenerateCenter();
        io.AddMousePosEvent(pos.x,pos.y);io.AddMouseButtonEvent(0,testFrame%2==0);
    }
    if(testFrame==38 || testFrame==39 || testFrame==42 || testFrame==43) {
        auto pos=testFrame<40?editor.GiDemoCenter():editor.LodDemoCenter();
        io.AddMousePosEvent(pos.x,pos.y);io.AddMouseButtonEvent(0,testFrame%2==0);
    }
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
        std::cout<<"PASS: five UI material selections, pause button, and resized editor rendering.\n";
    }
    if(testFrame==40) {
        if(s.objects.size()!=3 || !s.voxelGI) throw std::runtime_error("GI demo button failed");
        if(!SaveFramebufferPng("research-ui.png",fw,fh)) throw std::runtime_error("Research UI capture failed");
    }
    if(testFrame==44) {
        if(s.objects.size()!=64 || !s.clusterLOD || !s.clusterCulling) throw std::runtime_error("LOD demo button failed");
        std::cout<<"PASS: research GI and 64-object LOD scene buttons.\n";
    }
    if(testFrame==52) {
        if(s.objects.size()!=1000 || s.voxelGI || s.voxelShadows) throw std::runtime_error("Stress generation button failed");
        if(!SaveFramebufferPng("stress-ui.png",fw,fh) || glGetError()!=GL_NO_ERROR) throw std::runtime_error("Stress capture failed");
        std::cout<<"PASS: stress UI generates 1000 objects.\n";
    }
    if(testFrame==62 && s.Selected().effect!=5) throw std::runtime_error("Phong UI selection failed");
    if(testFrame==72) {
        if(s.light.enabled) throw std::runtime_error("Direct light toggle failed");
    }
    for(int axis=0;axis<3;++axis) {
        int start=80+axis*6;
        if(testFrame==start-1) {
            beforeLight={s.light.position[0],s.light.position[1],s.light.position[2]};beforeCamera=s.camera.Position();
        }
        if(testFrame==start+3) {
            const float previous[]={beforeLight.x,beforeLight.y,beforeLight.z};
            for(int i=0;i<3;++i) {
                float delta=std::abs(previous[i]-s.light.position[i]);
                if((i==axis && delta<0.01f) || (i!=axis && delta>1e-5f)) throw std::runtime_error("Gizmo axis constraint failed");
            }
            auto eye=s.camera.Position();
            if(eye.x!=beforeCamera.x || eye.y!=beforeCamera.y || eye.z!=beforeCamera.z) throw std::runtime_error("Gizmo drag orbited camera");
        }
    }
    if(testFrame==100) beforeLight={s.light.position[0],s.light.position[1],s.light.position[2]};
    if(testFrame==108) {
        if(s.light.position[0]!=beforeLight.x || s.light.position[1]!=beforeLight.y || s.light.position[2]!=beforeLight.z)
            throw std::runtime_error("Escape did not cancel light movement");
        if(!s.light.enabled) throw std::runtime_error("Light did not turn back on");
        if(!SaveFramebufferPng("light-editor-ui.png",fw,fh) || glGetError()!=GL_NO_ERROR) throw std::runtime_error("Light UI capture failed");
        glfwSetWindowShouldClose(window,GLFW_TRUE);
        std::cout<<"PASS: Phong model, Light tab, direct light toggle, XYZ gizmo drags without camera movement.\n";
    }
    ++testFrame;
}
