// 실험 선택 시 적절한 메시·색상·카메라 구도를 설정합니다.
#include "SceneSettings.h"

// 선택된 메시 크기에 맞춰 카메라를 초기 구도로 되돌립니다.
void FrameCamera(Settings& s) {
    s.camera.Frame(s.mesh == 2);
}
// 효과 번호: 0 조명, 1 툰, 2 프레넬, 3 디졸브, 4 물결. 물결은 평면을 기본 메시로 사용합니다.
void Preset(Settings& s,int effect) {
    s.effect=effect;
    s.mesh=effect==4?2:0;
    FrameCamera(s);
    s.color[0]=effect==3?0.65f:0.22f; s.color[1]=effect==3?0.31f:0.65f; s.color[2]=effect==3?0.15f:0.83f;
}
