// 객체별 재질·변환과 장면 공통 조명·카메라를 분리합니다.
#pragma once
#include "Light.h"
#include <vector>
#include <cstddef>

struct SceneObject {
    int id = 1;
    int effect = 0, mesh = 0;
    float color[3] = {0.22f, 0.65f, 0.83f};
    float roughness = 0.32f, rim = 2.6f, dissolve = 0.3f, wave = 0.16f;
    float position[3] = {0, 0, 0};
    float scale = 1.0f; // 양수 균일 스케일: 법선 방향은 변하지 않습니다.
    float emission = 0.0f;
};

struct Settings {
    static constexpr std::size_t MaxObjects = 4096;
    std::vector<SceneObject> objects{SceneObject{}};
    std::size_t selected = 0;
    int nextId = 2;
    LightSettings light;
    float ambient = 0.15f;
    OrbitCamera camera;
    bool animate = true, wireframe = false, floor = true;

    // 연구용 기능. UE Lumen/Nanite의 완전한 구현이 아닙니다.
    bool voxelGI = false, voxelShadows = false;
    float giIntensity = 1.5f;
    bool clusterLOD = false, clusterCulling = false;
    float lodPixels = 2.0f;
    int researchView = 0; // 0 최종 색, 1 간접광만, 2 클러스터 색, 3 LOD 색

    SceneObject& Selected() { return objects.at(selected); }
    const SceneObject& Selected() const { return objects.at(selected); }
    bool AddObject(int mesh);
    bool DuplicateSelected();
    bool RemoveSelected(); // Inspector가 항상 유효하도록 마지막 객체 하나는 유지합니다.
};
void FrameCamera(Settings& settings);
void Preset(Settings& settings, int effect);
void LoadLightingDemo(Settings& settings);
void LoadGeometryDemo(Settings& settings);
// 현재 선택 재질과 연구 옵션을 유지하면서 격자 장면을 생성합니다. mesh 3은 구/큐브 혼합입니다.
void GenerateStressScene(Settings& settings, int count, int mesh, int columns, float spacing, bool volume);
