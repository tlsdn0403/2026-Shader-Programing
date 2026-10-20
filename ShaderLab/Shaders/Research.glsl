// Voxel cone tracing 학습용 공통 코드. ShaderLibrary가 #include를 확장합니다.
// 참고: Crassin et al., Interactive Indirect Illumination Using Voxel Cone Tracing (2011).
uniform sampler3D uVoxelRadiance;
uniform bool uGIEnabled, uVoxelShadows;
uniform float uGIIntensity, uVoxelSize, uVolumeExtent, uEmission;
uniform vec3 uVolumeMin, uDebugColor;
uniform int uResearchView;

bool insideVolume(vec3 uv) { return all(greaterThanEqual(uv,vec3(0))) && all(lessThanEqual(uv,vec3(1))); }
vec3 traceCone(vec3 origin, vec3 direction) {
    vec3 sum=vec3(0); float alpha=0.0, distance=uVoxelSize*1.5;
    for(int i=0;i<28;++i) {
        float diameter=max(uVoxelSize,distance*0.65);
        vec3 uv=(origin+direction*distance-uVolumeMin)/uVolumeExtent;
        if(!insideVolume(uv) || alpha>0.97) break;
        vec4 voxel=textureLod(uVoxelRadiance,uv,log2(diameter/uVoxelSize));
        // mip의 RGB는 빈 셀과 평균된 방사휘도이므로 alpha를 다시 곱하지 않습니다.
        sum+=(1.0-alpha)*voxel.rgb;
        alpha+=(1.0-alpha)*voxel.a;
        distance+=diameter*0.75;
    }
    return sum;
}
vec3 indirectLight(vec3 p,vec3 n) {
    if(!uGIEnabled) return vec3(0);
    vec3 uv=(p-uVolumeMin)/uVolumeExtent;
    if(!insideVolume(uv)) return vec3(0);
    vec3 tangent=normalize(cross(n,abs(n.y)<0.95?vec3(0,1,0):vec3(1,0,0)));
    vec3 bitangent=cross(n,tangent);
    vec3 origin=p+n*uVoxelSize*1.6;
    vec3 value=traceCone(origin,n)*0.25;
    value+=traceCone(origin,normalize(n+tangent))*0.1875;
    value+=traceCone(origin,normalize(n-tangent))*0.1875;
    value+=traceCone(origin,normalize(n+bitangent))*0.1875;
    value+=traceCone(origin,normalize(n-bitangent))*0.1875;
    return value*uGIIntensity;
}
float directVisibility(vec3 p,vec3 n,vec3 light) {
    if(!uVoxelShadows) return 1.0;
    vec3 origin=p+n*uVoxelSize*1.6;
    vec3 delta=uLightType==0 ? -uLightDirection*(uVolumeExtent*2.0) : light-origin;
    float lengthToLight=length(delta), visibility=1.0;
    vec3 direction=delta/max(lengthToLight,0.001);
    for(int i=1;i<96;++i) {
        float distance=float(i)*uVoxelSize;
        if(distance>=lengthToLight) break;
        vec3 uv=(origin+direction*distance-uVolumeMin)/uVolumeExtent;
        if(!insideVolume(uv)) break;
        visibility*=1.0-textureLod(uVoxelRadiance,uv,0.0).a;
        if(visibility<0.03) return 0.0;
    }
    return visibility;
}
