// 카메라 수학: 월드 공간을 카메라 공간으로 옮긴 뒤 원근 투영합니다.
#include "Camera.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float pi = 3.14159265359f;
Vec3 Sub(Vec3 a, Vec3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
Vec3 Cross(Vec3 a, Vec3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
float Dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
Vec3 Unit(Vec3 v) { float d=std::sqrt(Dot(v,v)); return {v.x/d,v.y/d,v.z/d}; }
using Matrix4 = std::array<float,16>;
// OpenGL에 전달할 행렬은 열 우선 배열입니다. 결과는 a × b입니다.
Matrix4 Multiply(const Matrix4& a, const Matrix4& b) {
    Matrix4 r{};
    for(int c=0;c<4;++c) for(int row=0;row<4;++row) for(int k=0;k<4;++k)
        r[c*4+row]+=a[k*4+row]*b[c*4+k];
    return r;
}
// 원점을 바라보는 뷰 행렬과 수직 시야각 45도, near 0.1 / far 100의 원근 투영을 결합합니다.
Matrix4 BuildViewProjection(Vec3 eye, float aspect) {
    Vec3 f=Unit(Sub({0,0,0},eye)), s=Unit(Cross(f,{0,1,0})), u=Cross(s,f);
    Matrix4 view={s.x,u.x,-f.x,0, s.y,u.y,-f.y,0, s.z,u.z,-f.z,0, -Dot(s,eye),-Dot(u,eye),Dot(f,eye),1};
    float t=1.0f/std::tan(pi/8.0f);
    Matrix4 projection={t/aspect,0,0,0, 0,t,0,0, 0,0,-100.1f/99.9f,-1, 0,0,-20.0f/99.9f,0};
    return Multiply(projection,view);
}
}
// 넓은 평면은 카메라를 더 멀리 배치하고 위에서 보도록 기본 구도를 맞춥니다.
void OrbitCamera::Frame(bool largePlane) {
    yaw = 0.55f;
    pitch = largePlane ? 0.65f : 0.28f;
    distance = largePlane ? 11.0f : 5.0f;
}
// 마우스 이동량을 회전각에 반영합니다. 수직 회전은 뒤집힘을 피하도록 제한합니다.
void OrbitCamera::Orbit(float deltaX, float deltaY) {
    yaw -= deltaX * 0.008f;
    pitch = std::clamp(pitch + deltaY * 0.008f, -1.3f, 1.3f);
}
// 휠 입력으로 원점과의 거리를 조절합니다.
void OrbitCamera::Zoom(float wheel) {
    distance = std::clamp(distance - wheel * 0.35f, 2.0f, 14.0f);
}
// 구면 좌표(수평각·수직각·거리)를 월드 공간의 xyz 좌표로 변환합니다.
Vec3 OrbitCamera::Position() const {
    return {distance * std::cos(pitch) * std::sin(yaw), distance * std::sin(pitch),
            distance * std::cos(pitch) * std::cos(yaw)};
}
Matrix4 OrbitCamera::ViewProjection(float aspect) const {
    return BuildViewProjection(Position(), std::max(aspect, 0.001f));
}
