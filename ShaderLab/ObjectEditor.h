// 선택 객체의 월드 XYZ 이동 및 Alt 드래그 복제를 담당합니다.
#pragma once
#include "SceneSettings.h"
#include <imgui.h>
class ObjectEditor {
public:
    bool DrawGizmo(Settings& s, ImVec2 origin, ImVec2 size, bool hovered);
    enum Mode {Move,Rotate,Scale};
    void SetMode(Mode next) {if(dragAxis<0) mode=next;}
    Mode GetMode() const {return mode;}
    bool Active() const {return dragAxis>=0;}
    void Show(bool show) {visible=show;if(!show)Cancel();}
    bool Visible() const {return visible;}
    void Cancel() {dragAxis=-1;}
    ImVec2 AxisCenter(int axis) const {return axisCenters[axis];}
private:
    Mode mode=Move;
    bool visible=false;
    float startRotation[3]{},startScale[3]{};
    ImVec2 startPoint{},dragDirection{};
    int dragAxis=-1,dragComponent=0,dragId=0;
    bool duplicatePending=false,duplicated=false;
    std::size_t originalIndex=0;
    float startPosition[3]{},startMouse=0;
    Matrix4 dragMatrix{};
    ImVec2 dragSize{},axisCenters[3]{};
};
