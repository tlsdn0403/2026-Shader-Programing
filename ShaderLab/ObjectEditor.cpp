#include "ObjectEditor.h"
#include <algorithm>
#include <cmath>

bool ObjectEditor::DrawGizmo(Settings& s,ImVec2 origin,ImVec2 size,bool hovered) {
    auto& io=ImGui::GetIO();
    if(!visible) return false;
    if(size.x<1 || size.y<1) {Cancel();return false;}
    // 선택 변경 시 이전 객체의 드래그를 이어받지 않습니다.
    if(dragAxis>=0 && s.Selected().id!=dragId) Cancel();
    bool captured=dragAxis>=0;
    if(dragAxis>=0) {
        // 드래그 시작 시 투영을 고정하고 clip 성분/w 식을 축 이동량에 대해 풉니다.
        // 원근에 따라 핸들이 마우스에서 미끄러지는 단순 픽셀/거리 비례 오차를 피합니다.
        if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if(duplicated) {s.RemoveSelected();s.selected=originalIndex;}
            else {
                std::copy(startPosition,startPosition+3,s.Selected().position);
                std::copy(startRotation,startRotation+3,s.Selected().rotation);
                std::copy(startScale,startScale+3,s.Selected().axisScale);
            }
            Cancel();
        } else if(!ImGui::IsMouseDown(0) || size.x!=dragSize.x || size.y!=dragSize.y) Cancel();
        else {
            // 클릭만으로 복제하지 않고 실제 이동이 시작될 때 한 번만 복제합니다.
            if(duplicatePending) {
                if(!ImGui::IsMouseDragging(0)) return true;
                duplicatePending=false;
                if(!s.DuplicateSelected(true)) {Cancel();return true;}
                duplicated=true;dragId=s.Selected().id;
            }
            if(mode!=Move) {
                float pixels=(io.MousePos.x-startPoint.x)*dragDirection.x+(io.MousePos.y-startPoint.y)*dragDirection.y;
                if(mode==Rotate) s.Selected().rotation[dragAxis]=std::remainder(startRotation[dragAxis]+pixels*0.8f,360.f);
                else s.Selected().axisScale[dragAxis]=std::clamp(startScale[dragAxis]*std::exp(std::clamp(pixels*0.01f,-10.f,10.f)),0.01f,100.f);
            } else {
            float w=dragMatrix[3]*startPosition[0]+dragMatrix[7]*startPosition[1]+dragMatrix[11]*startPosition[2]+dragMatrix[15];
            int c=dragComponent;
            float clip=dragMatrix[c]*startPosition[0]+dragMatrix[4+c]*startPosition[1]+dragMatrix[8+c]*startPosition[2]+dragMatrix[12+c];
            float mouse=c==0?io.MousePos.x:io.MousePos.y;
            float ndc=clip/w+(mouse-startMouse)*2/(c==0?size.x:-size.y);
            float denominator=ndc*dragMatrix[dragAxis*4+3]-dragMatrix[dragAxis*4+c];
            if(std::abs(denominator)>1e-6f) {
                float t=(clip-ndc*w)/denominator;
                if(std::isfinite(t) && w+t*dragMatrix[dragAxis*4+3]>0.1f)
                    s.Selected().position[dragAxis]=std::clamp(startPosition[dragAxis]+t,-10000.f,10000.f);
            }
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
    Vec3 p={s.Selected().position[0],s.Selected().position[1],s.Selected().position[2]};
    ImVec2 center;
    if(!project(p,center) || center.x<origin.x || center.x>origin.x+size.x || center.y<origin.y || center.y>origin.y+size.y) {
        draw->AddText({origin.x+10,origin.y+10},IM_COL32(255,220,110,255),"Object off-screen: press Home to frame scene");
        draw->PopClipRect();return captured;
    }
    float w=vp[3]*p.x+vp[7]*p.y+vp[11]*p.z+vp[15];
    float axisLength=w*0.828427f*85/size.y;
    Vec3 axes[]={{1,0,0},{0,1,0},{0,0,1}};
    auto rotation=s.Selected().RotationMatrix();
    // 스케일은 객체 로컬 축, 이동은 월드 축입니다.
    if(mode==Scale) for(int i=0;i<3;++i) axes[i]={rotation[i*3],rotation[i*3+1],rotation[i*3+2]};
    const ImU32 colors[]={IM_COL32(255,90,90,255),IM_COL32(90,240,130,255),IM_COL32(90,160,255,255)};
    const char* labels[]={"X","Y","Z"};
    ImVec2 ends[3]{};bool valid[3]{};
    int hot=-1;float nearest=100;
    ImVec2 tangent{1,0};
    auto hitSegment=[&](int axis,ImVec2 a,ImVec2 b) {
        float dx=b.x-a.x,dy=b.y-a.y,len2=dx*dx+dy*dy;
        if(len2<0.001f)return;
        float t=std::clamp(((io.MousePos.x-a.x)*dx+(io.MousePos.y-a.y)*dy)/len2,0.f,1.f);
        float x=io.MousePos.x-a.x-t*dx,y=io.MousePos.y-a.y-t*dy,d=x*x+y*y;
        if(hovered && d<nearest) {nearest=d;hot=axis;float len=std::sqrt(len2);tangent={dx/len,dy/len};}
    };
    ImVec2 rings[3][65]{};bool ringValid[3][65]{};
    for(int i=0;i<3;++i) {
        valid[i]=project({p.x+axes[i].x*axisLength,p.y+axes[i].y*axisLength,p.z+axes[i].z*axisLength},ends[i]);
        if(mode==Rotate) {
            // 축에 수직인 회전 고리를 선분으로 투영해 마우스 적중 검사를 합니다.
            for(int j=0;j<=64;++j) {
                float angle=j*6.2831853f/64;
                // 각 오일러 성분의 실제 회전축(Rz*Ry*Rx)에 맞는 고리입니다.
                Vec3 a,b;
                if(i==0) {a={rotation[3],rotation[4],rotation[5]};b={rotation[6],rotation[7],rotation[8]};}
                else if(i==1) {float z=s.Selected().rotation[2]*0.01745329252f;a={0,0,1};b={std::cos(z),std::sin(z),0};}
                else {a={1,0,0};b={0,1,0};}
                float c=axisLength*std::cos(angle),d=axisLength*std::sin(angle);
                Vec3 q={p.x+a.x*c+b.x*d,p.y+a.y*c+b.y*d,p.z+a.z*c+b.z*d};
                ringValid[i][j]=project(q,rings[i][j]);
                if(j && ringValid[i][j-1] && ringValid[i][j]) hitSegment(i,rings[i][j-1],rings[i][j]);
            }
            axisCenters[i]=rings[i][8];valid[i]=true;
        } else {
            float dx=ends[i].x-center.x,dy=ends[i].y-center.y,len2=dx*dx+dy*dy;
            if(!valid[i] || len2<144) {valid[i]=false;axisCenters[i]={};continue;}
            axisCenters[i]={center.x+dx*0.75f,center.y+dy*0.75f};
            hitSegment(i,{center.x+dx*0.18f,center.y+dy*0.18f},ends[i]);
        }
    }
    if(hot>=0 && ImGui::IsMouseClicked(0) && dragAxis<0) {
        duplicatePending=io.KeyAlt && mode==Move;duplicated=false;originalIndex=s.selected;dragId=s.Selected().id;
        dragAxis=hot;dragComponent=std::abs(ends[hot].x-center.x)>std::abs(ends[hot].y-center.y)?0:1;
        startPoint=io.MousePos;dragDirection=tangent;
        std::copy(s.Selected().rotation,s.Selected().rotation+3,startRotation);
        std::copy(s.Selected().axisScale,s.Selected().axisScale+3,startScale);
        std::copy(s.Selected().position,s.Selected().position+3,startPosition);
        startMouse=dragComponent==0?io.MousePos.x:io.MousePos.y;dragMatrix=vp;dragSize=size;captured=true;
    }
    for(int i=0;i<3;++i) if(valid[i]) {
        ImU32 color=(hot==i || dragAxis==i)?IM_COL32(255,240,100,255):colors[i];
        if(mode==Rotate) {
            for(int j=1;j<=64;++j) if(ringValid[i][j-1] && ringValid[i][j]) draw->AddLine(rings[i][j-1],rings[i][j],color,2.5f);
            draw->AddText(axisCenters[i],color,labels[i]);
        } else {
            draw->AddLine(center,ends[i],color,3);
            if(mode==Scale) draw->AddRectFilled({ends[i].x-5,ends[i].y-5},{ends[i].x+5,ends[i].y+5},color);
            else draw->AddCircleFilled(ends[i],5,color);
            draw->AddText({ends[i].x+6,ends[i].y-8},color,labels[i]);
        }
    }
    draw->AddCircleFilled(center,7,IM_COL32(255,220,90,255));
    draw->AddText({center.x+9,center.y+9},IM_COL32(255,240,190,255),mode==Move?"Move (W)":mode==Rotate?"Rotate (E)":"Scale (R)");
    if(hot>=0 || dragAxis>=0) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    draw->PopClipRect();return captured || hot>=0;
}
