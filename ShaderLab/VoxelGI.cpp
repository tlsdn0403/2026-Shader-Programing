#include "VoxelGI.h"
#include <algorithm>
#include <cmath>
#include <chrono>

namespace {
float Dot(Vec3 a,Vec3 b) {return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec3 Unit(Vec3 a) {float d=std::sqrt(std::max(Dot(a,a),1e-12f));return {a.x/d,a.y/d,a.z/d};}
// 단순 도형의 부호 있는 거리 근사입니다. 변형·discard 객체는 캐스터에서 제외합니다.
float Distance(const SceneObject& o, Vec3 p, Vec3& normal) {
    Vec3 q={(p.x-o.position[0])/o.scale,(p.y-o.position[1])/o.scale,(p.z-o.position[2])/o.scale};
    if(o.mesh==0) {normal=Unit(q);return (std::sqrt(Dot(q,q))-1)*o.scale;}
    if(o.mesh==1) {
        float x=std::abs(q.x)-0.8f,y=std::abs(q.y)-0.8f,z=std::abs(q.z)-0.8f;
        normal=x>=y&&x>=z?Vec3{q.x>=0?1.f:-1.f,0,0}:y>=z?Vec3{0,q.y>=0?1.f:-1.f,0}:Vec3{0,0,q.z>=0?1.f:-1.f};
        return std::max({x,y,z})*o.scale;
    }
    normal={0,q.y>=-1.2f?1.f:-1.f,0};
    return std::max({std::abs(q.x)-4, std::abs(q.z)-4, std::abs(q.y+1.2f)})*o.scale;
}
}

bool VoxelGI::Update(const Settings& s) {
    if(!s.voxelGI && !s.voxelShadows && s.researchView!=1) return false;
    // 카메라 이동은 캐시를 무효화하지 않습니다. 기하·색·발광·광원 변경만 재계산합니다.
    const float power=s.light.enabled?s.light.intensity:0.f;
    std::vector<float> key={static_cast<float>(s.light.type),power,s.floor?1.f:0.f,s.light.range,s.light.innerAngle,s.light.outerAngle};
    key.insert(key.end(),s.light.position,s.light.position+3);
    key.insert(key.end(),s.light.direction,s.light.direction+3);
    key.insert(key.end(),s.light.color,s.light.color+3);
    for(const auto& o:s.objects) {
        key.insert(key.end(),{static_cast<float>(o.mesh),static_cast<float>(o.effect),o.scale,o.emission});
        key.insert(key.end(),o.position,o.position+3); key.insert(key.end(),o.color,o.color+3);
    }
    if(texture && key==previousKey) return false;
    auto start=std::chrono::steady_clock::now();
    std::vector<SceneObject> sources;
    skipped=0;
    for(const auto& o:s.objects) {
        if(o.effect==3 || o.effect==4) {++skipped; continue;}
        sources.push_back(o);
    }
    if(s.floor) {
        SceneObject floor;floor.mesh=2;floor.position[1]=-0.02f;
        floor.color[0]=0.052f;floor.color[1]=0.067f;floor.color[2]=0.087f;sources.push_back(floor);
    }
    const int count=Resolution*Resolution*Resolution;
    std::vector<float> data(count*4,0);
    Vec3 low=Minimum();
    occupied=0;
    for(int z=0;z<Resolution;++z) for(int y=0;y<Resolution;++y) for(int x=0;x<Resolution;++x) {
        Vec3 p={low.x+(x+0.5f)*CellSize,low.y+(y+0.5f)*CellSize,low.z+(z+0.5f)*CellSize};
        // 얇은 표면을 놓치지 않도록 복셀 두께를 보수적으로 확장합니다.
        for(const auto& o:sources) {
            Vec3 n{};
            float d=Distance(o,p,n);
            if(d>CellSize*0.55f) continue;
            int i=((z*Resolution+y)*Resolution+x)*4;
            float direct=std::max(Dot(n,s.light.TowardLight(p)),0.f)*power*0.42f*s.light.Attenuation(p);
            // RGB는 방사휘도, A는 점유율입니다. 내부도 채워 광선이 뒷면으로 통과하지 않게 합니다.
            for(int c=0;c<3;++c) data[i+c]=o.color[c]*(direct*s.light.color[c]+o.emission);
            data[i+3]=1; ++occupied; break;
        }
    }
    if(!texture) glGenTextures(1,&texture);
    glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_3D,texture);
    glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA16F,Resolution,Resolution,Resolution,0,GL_RGBA,GL_FLOAT,data.data());
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER);
    const float border[4]={0,0,0,0};glTexParameterfv(GL_TEXTURE_3D,GL_TEXTURE_BORDER_COLOR,border);
    glGenerateMipmap(GL_TEXTURE_3D);glActiveTexture(GL_TEXTURE0);
    previousKey=key;++updates;
    updateMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    return true;
}
void VoxelGI::Bind(GLuint p,const Settings& s) const {
    auto loc=[&](const char* name){return glGetUniformLocation(p,name);};
    glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_3D,texture);glActiveTexture(GL_TEXTURE0);
    glUniform1i(loc("uVoxelRadiance"),3);
    glUniform1i(loc("uGIEnabled"),s.voxelGI || s.researchView==1);
    glUniform1i(loc("uVoxelShadows"),s.voxelShadows);
    glUniform1f(loc("uGIIntensity"),s.giIntensity);
    Vec3 low=Minimum();glUniform3f(loc("uVolumeMin"),low.x,low.y,low.z);
    glUniform1f(loc("uVolumeExtent"),Extent);glUniform1f(loc("uVoxelSize"),CellSize);
    glUniform1i(loc("uResearchView"),s.researchView);
}
void VoxelGI::Shutdown() {
    if(texture) glDeleteTextures(1,&texture);
    texture=0;previousKey.clear();occupied=skipped=0;updates=0;
}
