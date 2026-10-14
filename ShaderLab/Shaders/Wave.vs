#version 330 core
// 버텍스 셰이더: 정점 위치와 법선을 준비하고 월드 위치를 클립 공간으로 변환합니다.
// Wave vertex shader: animated displacement and corrected normals.
// 입력 위치 0은 정점 위치, 1은 법선입니다. Renderer의 VAO 설정과 일치해야 합니다.
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
uniform mat4 uViewProjection;
uniform bool uFloor;
uniform float uTime;
uniform float uWave;
// 다음 프래그먼트 단계로 전달되며 삼각형 내부에서 보간됩니다.
out vec3 vPosition;
out vec3 vNormal;
void main() {
    vec3 p = aPosition;
    vec3 n = aNormal;
    if (!uFloor) {
        // 시간에 따라 위상이 움직이는 두 파동을 합성해 정점 높이를 변경합니다.
        float a = p.x * 4.0 + uTime * 1.6;
        float b = p.z * 3.0 - uTime * 1.2;
        p.y += uWave * sin(a) * cos(b);
        // Inverse-transpose of the displacement Jacobian, valid for all meshes.
        // 변위의 x/z 기울기로 법선도 보정해야 움직인 표면에 맞는 조명이 계산됩니다.
        float dx = uWave * 4.0 * cos(a) * cos(b);
        float dz = -uWave * 3.0 * sin(a) * sin(b);
        n = normalize(vec3(n.x - dx*n.y, n.y, n.z - dz*n.y));
    }
    vPosition = p;
    vNormal = n;
    // 별도 모델 변환이 없어 현재 정점 좌표를 월드 좌표로 사용합니다. 뷰·투영 행렬만 곱합니다.
    gl_Position = uViewProjection * vec4(p, 1.0);
}
