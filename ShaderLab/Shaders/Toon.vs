#version 330 core
// 버텍스 셰이더: 정점 위치와 법선을 준비하고 월드 위치를 클립 공간으로 변환합니다.
// Toon vertex shader: world-space position and normal passed to the fragment shader.
// 입력 위치 0은 정점 위치, 1은 법선입니다. Renderer의 VAO 설정과 일치해야 합니다.
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
uniform mat4 uViewProjection;
uniform vec3 uPosition;
uniform float uScale;
// 다음 프래그먼트 단계로 전달되며 삼각형 내부에서 보간됩니다.
out vec3 vPosition;
out vec3 vNormal;
void main() {
    vec3 p = aPosition;
    vec3 n = aNormal;
    // 객체별 균일 크기와 위치를 적용한 월드 좌표입니다.
    vec3 worldPosition = p * uScale + uPosition;
    vPosition = worldPosition;
    vNormal = n;
    // 월드 좌표에 카메라의 뷰·투영 행렬을 적용합니다.
    gl_Position = uViewProjection * vec4(worldPosition, 1.0);
}
