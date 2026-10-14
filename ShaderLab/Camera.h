// 원점 주위를 도는 카메라. 회전각과 거리로 위치 및 뷰·투영 행렬을 계산합니다.
#pragma once
#include <array>
struct Vec3 { float x, y, z; };
using Matrix4 = std::array<float, 16>;

class OrbitCamera {
public:
    void Frame(bool largePlane);
    void Orbit(float deltaX, float deltaY);
    void Zoom(float wheel);
    Vec3 Position() const;
    Matrix4 ViewProjection(float aspect) const;
private:
    float yaw = 0.55f, pitch = 0.28f, distance = 5.0f;
};
