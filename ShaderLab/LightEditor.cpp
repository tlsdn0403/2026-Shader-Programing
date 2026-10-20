#include "LightEditor.h"
#include <algorithm>
#include <cmath>
#include <limits>
namespace {
Vec3 Add(Vec3 a,Vec3 b) {return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 Mul(Vec3 a,float s) {return {a.x*s,a.y*s,a.z*s};}
Vec3 Cross(Vec3 a,Vec3 b) {return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
float Length(Vec3 p) {return std::sqrt(p.x*p.x+p.y*p.y+p.z*p.z);}
Vec3 Unit(Vec3 p) {return Mul(p,1/std::max(Length(p),1e-6f));}
ImVec2 Center() {auto a=ImGui::GetItemRectMin(),b=ImGui::GetItemRectMax();return {(a.x+b.x)*0.5f,(a.y+b.y)*0.5f};}
}
void LightEditor::DrawPanel(Settings& s) {
    auto& light=s.light;
    // 재질 값은 선택 객체에만 적용하고, 아래 광원 값은 장면 전체에 적용합니다.
    ImGui::Text("SELECTED OBJECT %d",s.Selected().id);
    ImGui::TextDisabled("Material settings: selected object only");
    if(s.Selected().effect==0 || s.Selected().effect==5) {
        if(ImGui::RadioButton("Blinn-Phong (half vector)",s.Selected().effect==0)) s.Selected().effect=0;
        if(ImGui::RadioButton("Phong (reflection)",s.Selected().effect==5)) s.Selected().effect=5;
        phongCenter=Center();
    } else ImGui::TextWrapped("Choose Studio lighting on the left to compare Phong / Blinn-Phong.");
    ImGui::ColorEdit3("Base color",s.Selected().color,ImGuiColorEditFlags_NoInputs);
    ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Emission",&s.Selected().emission,0,8,"Emission %.2f");
    ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Roughness",&s.Selected().roughness,0.02f,1,"Specular width %.2f");
    if(s.Selected().effect==2) {
        ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Rim",&s.Selected().rim,0.3f,8,"Rim power %.2f");
    }
    ImGui::Separator();
    ImGui::TextUnformatted("SCENE LIGHT (all objects)");
    ImGui::Checkbox("Direct light enabled",&light.enabled);enabledCenter=Center();
    if(ImGui::Button("Frame light + selected",{-1,30})) {
        const auto& o=s.Selected();
        Vec3 p={light.position[0],light.position[1],light.position[2]},q={o.position[0],o.position[1],o.position[2]};
        s.camera.Frame(Mul(Add(p,q),0.5f),Length(Add(p,Mul(q,-1)))*0.5f+std::max(1.f,o.scale*(o.mesh==2?6.f:1.75f)));
    }
    frameCenter=Center();
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##LightType",&light.type,"Directional\0Point\0Spot\0");
    ImGui::ColorEdit3("Light color",light.color,ImGuiColorEditFlags_NoInputs);
    ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Intensity",&light.intensity,0,30,"Intensity %.2f");
    ImGui::TextUnformatted("Position X / Y / Z");
    ImGui::SetNextItemWidth(-1);ImGui::DragFloat3("##LightPosition",light.position,0.05f,-10000,10000,"%.2f");
    if(light.type==LightSettings::Directional)
        ImGui::TextWrapped("Directional position moves the handle only. Direction controls lighting.");
    ImGui::BeginDisabled(light.type==LightSettings::Point);
    ImGui::TextUnformatted("Direction X / Y / Z");
    ImGui::SetNextItemWidth(-1);ImGui::DragFloat3("##LightDirection",light.direction,0.01f,-1,1,"%.2f");
    if(ImGui::Button("Aim at selected object",{-1,28})) {
        const auto& object=s.Selected();
        Vec3 d=Unit({object.position[0]-light.position[0],object.position[1]-light.position[1],object.position[2]-light.position[2]});
        light.direction[0]=d.x;light.direction[1]=d.y;light.direction[2]=d.z;
    }
    ImGui::EndDisabled();
    ImGui::TextWrapped("Direction = light travel direction. Zero uses (0, -1, 0).");
    ImGui::BeginDisabled(light.type==LightSettings::Directional);
    ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##LightRange",&light.range,0.5f,100,"Range %.1f");
    ImGui::EndDisabled();
    if(light.type==LightSettings::Spot) {
        ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##OuterCone",&light.outerAngle,1,85,"Outer half-angle %.1f deg");
        light.innerAngle=std::min(light.innerAngle,light.outerAngle-0.1f);
        ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##InnerCone",&light.innerAngle,0,light.outerAngle-0.1f,"Inner half-angle %.1f deg");
    }
    ImGui::Separator();
    ImGui::Checkbox("Show XYZ handles",&showGizmo);
    ImGui::Checkbox("Show range / cone",&showBounds);

    ImGui::TextWrapped("Drag red X / green Y / blue Z in the viewport. Esc cancels. Drag empty space to orbit.");
    ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##Ambient",&s.ambient,0,0.6f,"Ambient fill %.2f");
    if(ImGui::CollapsingHeader("Indirect light / shadows")) {
        ImGui::Checkbox("Voxel cone tracing GI",&s.voxelGI);
        ImGui::Checkbox("Voxel direct shadows",&s.voxelShadows);
        ImGui::SetNextItemWidth(-1);ImGui::SliderFloat("##GIIntensity",&s.giIntensity,0,4,"Bounce strength %.2f");
        ImGui::TextWrapped("GI uses a fixed volume. Dissolve / Wave receive GI but do not cast it. Diagnostics are in Research.");
    }
}

bool LightEditor::DrawGizmo(Settings& s,ImVec2 origin,ImVec2 size,bool hovered) {
    auto& io=ImGui::GetIO();
    if(!showGizmo || size.x<1 || size.y<1) {Cancel();return false;}
    bool captured=dragAxis>=0;
    if(dragAxis>=0) {
        // 드래그 시작 시 투영을 고정하고 clip 성분/w 식을 축 이동량에 대해 풉니다.
        // 원근에 따라 핸들이 마우스에서 미끄러지는 단순 픽셀/거리 비례 오차를 피합니다.
        if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            std::copy(startPosition,startPosition+3,s.light.position);Cancel();
        } else if(!ImGui::IsMouseDown(0) || size.x!=dragSize.x || size.y!=dragSize.y) Cancel();
        else {
            float w=dragMatrix[3]*startPosition[0]+dragMatrix[7]*startPosition[1]+dragMatrix[11]*startPosition[2]+dragMatrix[15];
            int c=dragComponent;
            float clip=dragMatrix[c]*startPosition[0]+dragMatrix[4+c]*startPosition[1]+dragMatrix[8+c]*startPosition[2]+dragMatrix[12+c];
            float mouse=c==0?io.MousePos.x:io.MousePos.y;
            float ndc=clip/w+(mouse-startMouse)*2/(c==0?size.x:-size.y);
            float denominator=ndc*dragMatrix[dragAxis*4+3]-dragMatrix[dragAxis*4+c];
            if(std::abs(denominator)>1e-6f) {
                float t=(clip-ndc*w)/denominator;
                if(std::isfinite(t) && w+t*dragMatrix[dragAxis*4+3]>0.1f)
                    s.light.position[dragAxis]=std::clamp(startPosition[dragAxis]+t,-10000.f,10000.f);
            }
        }
    }
    Matrix4 vp=s.camera.ViewProjection(size.x/size.y);
    auto project=[&](Vec3 p,ImVec2& out) {
        float w=vp[3]*p.x+vp[7]*p.y+vp[11]*p.z+vp[15];
        if(w<=0.1f) return false;
        float x=(vp[0]*p.x+vp[4]*p.y+vp[8]*p.z+vp[12])/w;
        float y=(vp[1]*p.x+vp[5]*p.y+vp[9]*p.z+vp[13])/w;
        out={origin.x+(x+1)*size.x*0.5f,origin.y+(1-y)*size.y*0.5f};return true;
    };
    auto* draw=ImGui::GetWindowDrawList();
    draw->PushClipRect(origin,{origin.x+size.x,origin.y+size.y},true);
    Vec3 p={s.light.position[0],s.light.position[1],s.light.position[2]};
    ImVec2 center;
    if(!project(p,center) || center.x<origin.x || center.x>origin.x+size.x || center.y<origin.y || center.y>origin.y+size.y) {
        draw->AddText({origin.x+10,origin.y+10},IM_COL32(255,220,110,255),"Light off-screen: use 'Frame light + selected'");
        draw->PopClipRect();return captured;
    }
    float w=vp[3]*p.x+vp[7]*p.y+vp[11]*p.z+vp[15];
    float axisLength=w*0.828427f*85/size.y;
    const Vec3 axes[]={{1,0,0},{0,1,0},{0,0,1}};
    const ImU32 colors[]={IM_COL32(255,90,90,255),IM_COL32(90,240,130,255),IM_COL32(90,160,255,255)};
    const char* labels[]={"X","Y","Z"};
    ImVec2 ends[3]{};bool valid[3]{};
    int hot=-1;float nearest=100;
    for(int i=0;i<3;++i) {
        valid[i]=project(Add(p,Mul(axes[i],axisLength)),ends[i]);
        float dx=ends[i].x-center.x,dy=ends[i].y-center.y, len2=dx*dx+dy*dy;
        if(!valid[i] || len2<144) {valid[i]=false;axisCenters[i]={};continue;}
        axisCenters[i]={center.x+dx*0.75f,center.y+dy*0.75f};
        float t=std::clamp(((io.MousePos.x-center.x)*dx+(io.MousePos.y-center.y)*dy)/len2,0.18f,1.f);
        float x=io.MousePos.x-center.x-t*dx,y=io.MousePos.y-center.y-t*dy, distance=x*x+y*y;
        if(hovered && distance<nearest) {nearest=distance;hot=i;}
    }
    if(hot>=0 && ImGui::IsMouseClicked(0) && dragAxis<0) {
        dragAxis=hot;dragComponent=std::abs(ends[hot].x-center.x)>std::abs(ends[hot].y-center.y)?0:1;
        std::copy(s.light.position,s.light.position+3,startPosition);
        startMouse=dragComponent==0?io.MousePos.x:io.MousePos.y;dragMatrix=vp;dragSize=size;captured=true;
    }
    for(int i=0;i<3;++i) if(valid[i]) {
        ImU32 color=(hot==i || dragAxis==i)?IM_COL32(255,240,100,255):colors[i];
        draw->AddLine(center,ends[i],color,3);
        draw->AddCircleFilled(ends[i],5,color);
        draw->AddText({ends[i].x+6,ends[i].y-8},color,labels[i]);
    }
    draw->AddCircleFilled(center,7,IM_COL32(255,220,90,255));
    draw->AddText({center.x+9,center.y+9},IM_COL32(255,240,190,255),"Light");
    auto line=[&](Vec3 a,Vec3 b) {ImVec2 x,y;if(project(a,x)&&project(b,y))draw->AddLine(x,y,IM_COL32(255,220,90,150),1);};
    Vec3 direction=s.light.Direction();
    if(s.light.type!=LightSettings::Point) line(p,Add(p,Mul(direction,axisLength*2)));
    if(showBounds && s.light.type!=LightSettings::Directional) {
        if(s.light.type==LightSettings::Spot) {
            Vec3 t=Unit(Cross(direction,std::abs(direction.y)<0.95f?Vec3{0,1,0}:Vec3{1,0,0}));
            Vec3 b=Cross(direction,t);
            float angle=s.light.outerAngle*0.01745329252f;
            Vec3 end=Add(p,Mul(direction,s.light.range*std::cos(angle)));
            float radius=s.light.range*std::sin(angle);
            for(int i=0;i<48;++i) {
                auto point=[&](int j){float a=j*6.2831853f/48;return Add(end,Add(Mul(t,radius*std::cos(a)),Mul(b,radius*std::sin(a))));};
                line(point(i),point(i+1));if(i%12==0)line(p,point(i));
            }
        } else for(int plane=0;plane<3;++plane) for(int i=0;i<48;++i) {
            auto point=[&](int j){float a=j*6.2831853f/48;return Add(p,Add(Mul(axes[plane],s.light.range*std::cos(a)),Mul(axes[(plane+1)%3],s.light.range*std::sin(a))));};
            line(point(i),point(i+1));
        }
    }
    if(hot>=0 || dragAxis>=0) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    draw->PopClipRect();return captured || hot>=0;
}
