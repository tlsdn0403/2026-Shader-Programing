// 0: 방향광, 1: 점광원, 2: 스포트라이트. CPU LightSettings와 같은 감쇠식입니다.
uniform int uLightType;
uniform vec3 uLight, uLightDirection, uLightColor;
uniform float uPower, uLightRange, uSpotInnerCos, uSpotOuterCos;
vec3 safeUnit(vec3 v,vec3 fallback) {return dot(v,v)>1e-12?v*inversesqrt(dot(v,v)):fallback;}
vec3 lightVector(vec3 p) {
    if(uLightType==0) return -uLightDirection;
    return safeUnit(uLight-p,vec3(0,1,0));
}
vec3 lightRadiance(vec3 p) {
    float attenuation=1.0;
    if(uLightType!=0) {
        float distance=length(uLight-p);
        float fade=max(0.0,1.0-distance/uLightRange);
        attenuation=fade*fade/(1.0+0.09*distance+0.032*distance*distance);
        if(uLightType==2)
            attenuation*=smoothstep(uSpotOuterCos,uSpotInnerCos,dot(-lightVector(p),uLightDirection));
    }
    return uLightColor*uPower*attenuation;
}
