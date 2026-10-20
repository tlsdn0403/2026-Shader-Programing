// Light 탭과 월드 XYZ 이동 핸들. 편집기 오버레이이므로 렌더링 캡처에는 포함하지 않습니다.
#pragma once
#include "SceneSettings.h"
#include <imgui.h>
class LightEditor {
public:
    void DrawPanel(Settings& settings);
    bool DrawGizmo(Settings& settings,ImVec2 topLeft,ImVec2 size,bool hovered);
    void Cancel() {dragAxis=-1;}
    ImVec2 PhongCenter() const {return phongCenter;}
    ImVec2 EnabledCenter() const {return enabledCenter;}
    ImVec2 FrameCenter() const {return frameCenter;}
    ImVec2 AxisCenter(int axis) const {return axisCenters[axis];}
private:
    bool showGizmo=true,showBounds=false;
    int dragAxis=-1,dragComponent=0;
    float startPosition[3]{},startMouse=0;
    Matrix4 dragMatrix{};
    ImVec2 phongCenter{},dragSize{},enabledCenter{},frameCenter{},axisCenters[3]{};
};
