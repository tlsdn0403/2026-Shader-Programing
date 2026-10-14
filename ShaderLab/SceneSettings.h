// UI와 렌더러가 공유하는 편집 상태입니다. GPU 자원을 직접 소유하지 않습니다.
#pragma once
#include "Camera.h"

struct Settings {
    int effect = 0, mesh = 0;
    float color[3] = {0.22f, 0.65f, 0.83f};
    float roughness = 0.32f, rim = 2.6f, dissolve = 0.3f, wave = 0.16f;
    float lightAngle = 0.7f, lightPower = 3.0f, ambient = 0.15f;
    OrbitCamera camera;
    bool animate = true, wireframe = false, floor = true;
};

void FrameCamera(Settings& settings);
void Preset(Settings& settings, int effect);
