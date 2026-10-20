#version 330 core
#include "Light.glsl"
#include "Research.glsl"
// 물결 표면 조명: 버텍스 단계에서 변형·보정한 위치와 법선으로 조명과 림 효과를 계산합니다.
// Wave fragment shader. uFloor keeps the reference grid unaffected by the effect.
in vec3 vPosition;
in vec3 vNormal;
out vec4 fragColor;
uniform vec3 uBaseColor;
uniform vec3 uCamera;

uniform float uAmbient, uRoughness;
uniform float uRim;
uniform bool uFloor;
void main() {
    // N: 표면 법선, V: 표면에서 카메라 방향, L: 표면에서 광원 방향.
    vec3 N=normalize(vNormal), V=normalize(uCamera-vPosition), L=lightVector(vPosition);
    if (!gl_FrontFacing) N=-N;
    vec3 baseColor=uBaseColor;
    // 기준 바닥은 좌표 기반 격자로 표시합니다. fwidth로 화면상 선 두께를 보정합니다.
    if(uFloor) {
        vec2 grid=abs(fract(vPosition.xz)-0.5)/max(fwidth(vPosition.xz),vec2(0.001));
        float line=1.0-min(min(grid.x,grid.y),1.0);
        baseColor=mix(vec3(0.052,0.067,0.087),vec3(0.11,0.15,0.18),line*0.65);
    }
    // Lambert 확산광: 표면이 광원을 향할수록 밝고 반대쪽이면 0입니다.
    float diffuse=max(dot(N,L),0.0);
    // 여기서 roughness는 PBR 거칠기가 아니라 Blinn–Phong 하이라이트 폭을 조절합니다.
    float exponent=mix(180.0,4.0,uRoughness);
    // 빛과 시선의 중간 방향(half vector)을 이용해 반짝이는 하이라이트를 계산합니다.
    float spec=pow(max(dot(N,safeUnit(L+V,N)),0.0),exponent)*step(0.001,diffuse);
    // N·V가 작아지는 실루엣 부근에서 값이 커집니다. uRim으로 가장자리 폭을 조절합니다.
    float rim=pow(1.0-max(dot(N,V),0.0),uRim);
    // 일정한 주변광 + 직접 확산광 + 반사 하이라이트. 주변광은 실제 간접광(GI) 계산이 아닙니다.
    // 복셀 그림자는 별도 토글입니다. GI on/off 비교 시 직접광은 동일하게 유지할 수 있습니다.
    float visibility=directVisibility(vPosition,N,uLight);
    diffuse*=visibility; spec*=visibility;
    vec3 radiance=lightRadiance(vPosition); // 광원 색, 세기, 거리/원뿔 감쇠
    vec3 color=baseColor*uAmbient+(baseColor*diffuse*0.42+vec3(spec*0.3))*radiance;
    if(!uFloor) color+=rim*vec3(0.08,0.5,0.75);
    // 강한 밝기를 0~1 범위로 압축(톤 매핑)한 뒤 감마 2.2로 화면 표시용 색을 만듭니다.
    vec3 indirect=baseColor*indirectLight(vPosition,N);
    color+=indirect;
    if(!uFloor) color+=uBaseColor*uEmission;
    if(uResearchView==1) color=indirect;
    if((uResearchView==2 || uResearchView==3) && !uFloor) color=uDebugColor;
    color=color/(color+vec3(1.0));
    fragColor=vec4(pow(color,vec3(1.0/2.2)),1.0);
}
