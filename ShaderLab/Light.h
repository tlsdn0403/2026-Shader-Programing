// 광원의 상태와 CPU 복셀 조명 계산입니다. 같은 감쇠식을 Light.glsl에서도 사용합니다.
#pragma once
#include "Camera.h"

struct LightSettings {
    enum Type { Directional, Point, Spot };
    int type=Point;
    bool enabled=true;
    float color[3]={1,1,1};
    float intensity=3;
    float position[3]={3,4,2};
    float direction[3]={-0.557086f,-0.742781f,-0.371391f}; // 빛이 나아가는 방향
    float range=20;
    float innerAngle=20, outerAngle=35; // 원뿔 반각, degrees
    Vec3 Direction() const;
    Vec3 TowardLight(Vec3 surface) const;
    float Attenuation(Vec3 surface) const;
};
