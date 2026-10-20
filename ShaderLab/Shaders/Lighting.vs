#version 330 core

// 위치와 노멀값을 받음
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;

uniform mat4 uViewProjection;
uniform vec3 uPosition;
uniform float uScale;


// 프래그먼트 셰이더로 전달
out vec3 vPosition;
out vec3 vNormal;
void main() {

    // 월드 좌표계 변환
    vec3 worldPosition = aPosition * uScale + uPosition;
    vPosition = worldPosition;
    vNormal = aNormal;
    // 월드 좌표에 카메라의 뷰·투영 행렬을 적용
    gl_Position = uViewProjection * vec4(worldPosition, 1.0);
}
