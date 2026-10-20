#include "Light.h"
#include <algorithm>
#include <cmath>
namespace {
float Dot(Vec3 a,Vec3 b) {return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec3 Unit(Vec3 v,Vec3 fallback) {
    float length=std::sqrt(Dot(v,v));
    return length>1e-6f?Vec3{v.x/length,v.y/length,v.z/length}:fallback;
}
}
Vec3 LightSettings::Direction() const {return Unit({direction[0],direction[1],direction[2]},{0,-1,0});}
Vec3 LightSettings::TowardLight(Vec3 p) const {
    Vec3 d=Direction();
    if(type==Directional) return {-d.x,-d.y,-d.z};
    return Unit({position[0]-p.x,position[1]-p.y,position[2]-p.z},{0,1,0});
}
float LightSettings::Attenuation(Vec3 p) const {
    if(type==Directional) return 1;
    Vec3 delta={position[0]-p.x,position[1]-p.y,position[2]-p.z};
    float distance=std::sqrt(Dot(delta,delta));
    float fade=std::max(0.f,1-distance/std::max(range,0.1f));
    float attenuation=fade*fade/(1+0.09f*distance+0.032f*distance*distance);
    if(type==Spot) {
        Vec3 l=TowardLight(p), d=Direction();
        float inner=std::clamp(innerAngle,0.f,88.f);
        float outer=std::clamp(outerAngle,inner+0.1f,89.f);
        float lo=std::cos(outer*0.01745329252f),hi=std::cos(inner*0.01745329252f);
        float t=std::clamp((-Dot(l,d)-lo)/(hi-lo),0.f,1.f);
        attenuation*=t*t*(3-2*t);
    }
    return attenuation;
}
