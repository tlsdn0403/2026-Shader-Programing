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
// 대량 격자가 잘리지 않도록 카메라 거리에 맞춰 near/far 범위를 확장합니다.
Matrix4 BuildViewProjection(Vec3 eye, Vec3 target, float aspect) {
    Vec3 f=Unit(Sub(target,eye)), s=Unit(Cross(f,{0,1,0})), u=Cross(s,f);
    Matrix4 view={s.x,u.x,-f.x,0, s.y,u.y,-f.y,0, s.z,u.z,-f.z,0, -Dot(s,eye),-Dot(u,eye),Dot(f,eye),1};
    float t=1.0f/std::tan(pi/8.0f);
    Vec3 delta=Sub(eye,target);float distance=std::sqrt(Dot(delta,delta));
    float farPlane=std::max(1000.f,distance*3.f), nearPlane=std::max(0.1f,distance*0.0001f);
    Matrix4 projection={t/aspect,0,0,0, 0,t,0,0, 0,0,-(farPlane+nearPlane)/(farPlane-nearPlane),-1,
        0,0,-2*farPlane*nearPlane/(farPlane-nearPlane),0};
    return Multiply(projection,view);
}
}
// 넓은 평면은 카메라를 더 멀리 배치하고 위에서 보도록 기본 구도를 맞춥니다.
void OrbitCamera::Frame(Vec3 focus, float radius) {
    target = focus;
    bool largePlane = radius > 2;
    yaw = 0.55f;
    pitch = largePlane ? 0.65f : 0.28f;
    distance = std::clamp(radius * 3.5f, 5.0f, 200000.0f);
}
// 마우스 이동량을 회전각에 반영합니다. 수직 회전은 뒤집힘을 피하도록 제한합니다.
void OrbitCamera::Orbit(float deltaX, float deltaY) {
    yaw -= deltaX * 0.008f;
    pitch = std::clamp(pitch + deltaY * 0.008f, -1.3f, 1.3f);
}
// 휠 입력으로 원점과의 거리를 조절합니다.
void OrbitCamera::Zoom(float wheel) {
    distance = std::clamp(distance * std::exp(-wheel*0.07f), 2.0f, 200000.0f);
}
// 구면 좌표(수평각·수직각·거리)를 월드 공간의 xyz 좌표로 변환합니다.
Vec3 OrbitCamera::Position() const {
    return {target.x + distance * std::cos(pitch) * std::sin(yaw), target.y + distance * std::sin(pitch),
            target.z + distance * std::cos(pitch) * std::cos(yaw)};
}
Matrix4 OrbitCamera::ViewProjection(float aspect) const {
    return BuildViewProjection(Position(), target, std::max(aspect, 0.001f));
}
