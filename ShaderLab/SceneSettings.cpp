// 객체는 GPU 메시를 공유하지만 위치와 재질은 각각 독립적으로 가집니다.
#include "SceneSettings.h"
#include <algorithm>
#include <cmath>

bool Settings::AddObject(int mesh) {
    if(objects.size() >= MaxObjects) return false;
    SceneObject object;
    object.id = nextId++;
    object.mesh = std::clamp(mesh, 0, 2);
    if(object.mesh == 2) object.scale = 0.3f;
    // 기존 객체와 같은 위치에 겹치지 않는 3단위 격자 위치를 찾습니다.
    for(int slot=0; slot<256; ++slot) {
        const int columns[]={0,1,-1,2,-2,3,-3,4};
        float x = static_cast<float>(columns[slot % 8]) * 3.0f;
        float z = static_cast<float>(slot / 8) * 3.0f;
        bool occupied = false;
        for(const auto& other : objects)
            if(std::abs(other.position[0]-x)<2.5f && std::abs(other.position[2]-z)<2.5f) occupied=true;
        if(!occupied) { object.position[0]=x; object.position[2]=z; break; }
    }
    objects.push_back(object);
    selected = objects.size()-1;
    return true;
}
bool Settings::DuplicateSelected() {
    if(objects.size() >= MaxObjects) return false;
    SceneObject copy = Selected();
    if(!AddObject(copy.mesh)) return false;
    const SceneObject placement = Selected();
    copy.id = placement.id;
    copy.position[0] = placement.position[0];
    copy.position[2] = placement.position[2];
    objects[selected] = copy;
    return true;
}
bool Settings::RemoveSelected() {
    if(objects.size() <= 1) return false;
    objects.erase(objects.begin()+selected);
    selected = std::min(selected, objects.size()-1);
    return true;
}
// Home 및 추가 버튼은 전체 객체의 중심과 범위를 기준으로 카메라를 맞춥니다.
void FrameCamera(Settings& s) {
    Vec3 center{};
    for(const auto& o:s.objects) { center.x+=o.position[0]; center.y+=o.position[1]; center.z+=o.position[2]; }
    float count=static_cast<float>(s.objects.size());
    center.x/=count; center.y/=count; center.z/=count;
    float radius=1;
    for(const auto& o:s.objects) {
        float x=o.position[0]-center.x, y=o.position[1]-center.y, z=o.position[2]-center.z;
        radius=std::max(radius,std::sqrt(x*x+y*y+z*z)+(o.mesh==2?4.0f:1.0f)*o.scale);
    }
    s.camera.Frame(center,radius);
}
// 목록의 효과 변경은 선택된 객체에만 적용합니다. 위치·크기는 유지합니다.
void Preset(Settings& s,int effect) {
    auto& object=s.Selected();
    object.effect=effect;
    // 재질을 바꾸어도 사용자가 선택한 구/큐브/평면은 유지합니다.
    object.color[0]=effect==3?0.65f:0.22f;
    object.color[1]=effect==3?0.31f:0.65f;
    object.color[2]=effect==3?0.15f:0.83f;
    FrameCamera(s);
}
