// 기본 도형의 로컬 좌표로 UV를 만듭니다. LOD에서도 별도 UV 버퍼 없이 동작합니다.
in vec3 vTexturePosition;
in vec3 vTextureNormal;
uniform sampler2D uBaseTexture;
uniform bool uUseTexture, uTextureTint;
uniform int uTextureMesh;
uniform vec2 uTextureRepeat;
vec3 surfaceBaseColor(vec3 tint) {
    if(!uUseTexture) return tint;
    vec3 p=vTexturePosition;
    vec2 uv;
    if(uTextureMesh==0) {
        vec3 n=normalize(p);
        uv=vec2(atan(n.z,n.x)/6.2831853+0.5,acos(clamp(n.y,-1.0,1.0))/3.14159265);
    } else if(uTextureMesh==1) {
        // 각 면에 한 장씩 투영합니다. 구와 평면과 달리 큐브는 면별 UV입니다.
        vec3 n=vTextureNormal, a=abs(n);
        if(a.x>a.y && a.x>a.z) uv=vec2(-sign(n.x)*p.z,p.y)/1.6+0.5;
        else if(a.y>a.z) uv=vec2(p.x,-sign(n.y)*p.z)/1.6+0.5;
        else uv=vec2(sign(n.z)*p.x,p.y)/1.6+0.5;
        uv.y=1.0-uv.y;
    } else uv=vec2(p.x/8.0+0.5,0.5-p.z/8.0);
    uv*=uTextureRepeat;
    // 구의 경도 이음새에서 큰 미분값에 의해 mip가 과도하게 흐려지는 것을 방지합니다.
    vec2 dx=dFdx(uv),dy=dFdy(uv);
    if(uTextureMesh==0) {
        dx.x-=round(dx.x/uTextureRepeat.x)*uTextureRepeat.x;
        dy.x-=round(dy.x/uTextureRepeat.x)*uTextureRepeat.x;
    }
    vec3 color=textureGrad(uBaseTexture,uv,dx,dy).rgb; // sRGB 업로드로 선형 색 공간에서 조명 계산
    return color*(uTextureTint?tint:vec3(1.0));
}
