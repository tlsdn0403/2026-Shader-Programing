// 객체별 재질·변환과 장면 공통 조명·카메라를 분리합니다.
#pragma once
#include "Camera.h"
#include <vector>
#include <cstddef>

struct SceneObject {
    int id = 1;
    int effect = 0, mesh = 0;
    float color[3] = {0.22f, 0.65f, 0.83f};
    float roughness = 0.32f, rim = 2.6f, dissolve = 0.3f, wave = 0.16f;
    float position[3] = {0, 0, 0};
    float scale = 1.0f; // 양수 균일 스케일: 법선 방향은 변하지 않습니다.
};

struct Settings {
    static constexpr std::size_t MaxObjects = 64;
    std::vector<SceneObject> objects{SceneObject{}};
    std::size_t selected = 0;
    int nextId = 2;
    float lightAngle = 0.7f, lightPower = 3.0f, ambient = 0.15f;
    OrbitCamera camera;
    bool animate = true, wireframe = false, floor = true;

    SceneObject& Selected() { return objects.at(selected); }
    const SceneObject& Selected() const { return objects.at(selected); }
    bool AddObject(int mesh);
    bool DuplicateSelected();
    bool RemoveSelected(); // Inspector가 항상 유효하도록 마지막 객체 하나는 유지합니다.
};
void FrameCamera(Settings& settings);
void Preset(Settings& settings, int effect);
