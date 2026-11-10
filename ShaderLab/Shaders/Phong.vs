#version 330 core

// 위치와 노멀값을 받음
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;

uniform mat4 uViewProjection;
uniform vec3 uPosition;
// 비균일 스케일의 법선은 역전치 행렬로 변환합니다.
uniform mat3 uLinear, uNormalMatrix;


// 프래그먼트 셰이더로 전달
out vec3 vPosition;
out vec3 vNormal;
out vec3 vTexturePosition;
out vec3 vTextureNormal;
void main() {
    vTexturePosition=aPosition; vTextureNormal=aNormal;

    // 월드 좌표계 변환
    vec3 worldPosition = uLinear * aPosition + uPosition;
    vPosition = worldPosition;
    vNormal = normalize(uNormalMatrix * aNormal);
    // 월드 좌표에 카메라의 뷰·투영 행렬을 적용
    gl_Position = uViewProjection * vec4(worldPosition, 1.0);
}
