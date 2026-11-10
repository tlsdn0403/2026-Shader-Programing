// 객체는 GPU 메시를 공유하지만 위치와 재질은 각각 독립적으로 가집니다.
#include "SceneSettings.h"
#include <algorithm>
#include <cmath>
#include <set>

bool Settings::AddObject(int mesh) {
    if(objects.size() >= MaxObjects) return false;
    SceneObject object;
    object.id = nextId++;
    object.mesh = std::clamp(mesh, 0, 2);
    if(object.mesh == 2) object.scale = 0.3f;
    // 기존 중심 근처의 격자 칸을 한 번에 표시해 대량 객체에서도 반복 검색을 줄입니다.
    std::set<std::pair<int,int>> occupied;
    for(const auto& other:objects) {
        int cx=static_cast<int>(std::round(other.position[0]/3)), cz=static_cast<int>(std::round(other.position[2]/3));
        for(int x=cx-1;x<=cx+1;++x) for(int z=cz-1;z<=cz+1;++z)
            if(std::abs(other.position[0]-x*3.f)<2.5f && std::abs(other.position[2]-z*3.f)<2.5f) occupied.insert({x,z});
    }
    for(int slot=0;;++slot) {
        int col=slot%64, x=col%2?(col+1)/2:-col/2, z=slot/64;
        if(!occupied.count({x,z})) {object.position[0]=x*3.f;object.position[2]=z*3.f;break;}
    }
    objects.push_back(object);
    selected = objects.size()-1;
    return true;
}
bool Settings::DuplicateSelected(bool keepPosition) {
    if(objects.size() >= MaxObjects) return false;
    SceneObject copy = Selected();
    // Alt 드래그는 원본 위치에서 시작하며 새 ID만 부여합니다.
    if(keepPosition) {copy.id=nextId++;objects.push_back(copy);selected=objects.size()-1;return true;}
    if(!AddObject(copy.mesh)) return false;
    const SceneObject placement = Selected();
    copy.id = placement.id;
    copy.position[0] = placement.position[0];
    copy.position[2] = placement.position[2];
    objects[selected] = copy;
    return true;
}
bool Settings::RemoveSelected() {
    if(objects.size() <= 1) return false;
    objects.erase(objects.begin()+selected);
    selected = std::min(selected, objects.size()-1);
    return true;
}
// Home 및 추가 버튼은 전체 객체의 중심과 범위를 기준으로 카메라를 맞춥니다.
void FrameCamera(Settings& s) {
    Vec3 center{};
    for(const auto& o:s.objects) { center.x+=o.position[0]; center.y+=o.position[1]; center.z+=o.position[2]; }
    float count=static_cast<float>(s.objects.size());
    center.x/=count; center.y/=count; center.z/=count;
    float radius=1;
    for(const auto& o:s.objects) {
        float x=o.position[0]-center.x, y=o.position[1]-center.y, z=o.position[2]-center.z;
        radius=std::max(radius,std::sqrt(x*x+y*y+z*z)+(o.mesh==2?6.0f:1.75f)*o.MaxScale());
    }
    s.camera.Frame(center,radius);
}
// 목록의 효과 변경은 선택된 객체에만 적용합니다. 위치·크기는 유지합니다.
void Preset(Settings& s,int effect) {
    auto& object=s.Selected();
    object.effect=effect;
    // 재질을 바꾸어도 사용자가 선택한 구/큐브/평면은 유지합니다.
    object.color[0]=effect==3?0.65f:0.22f;
    object.color[1]=effect==3?0.31f:0.65f;
    object.color[2]=effect==3?0.15f:0.83f;
    FrameCamera(s);
}

// 발광 물체의 색이 주변 흰 물체와 바닥으로 전달되는 비교용 장면입니다.
void LoadLightingDemo(Settings& s) {
    s=Settings{};s.ambient=0.02f;s.light.intensity=1.2f;
    s.voxelGI=true;s.voxelShadows=true;s.giIntensity=2.0f;
    auto& receiver=s.objects[0];
    receiver.color[0]=receiver.color[1]=receiver.color[2]=0.75f;
    receiver.position[0]=0.4f;receiver.position[1]=-0.1f;
    SceneObject red;red.id=2;red.mesh=1;red.scale=0.85f;red.position[0]=-1.7f;red.position[1]=-0.5f;
    red.color[0]=1;red.color[1]=0.025f;red.color[2]=0.01f;red.emission=4;
    SceneObject blue=red;blue.id=3;blue.position[0]=1.7f;blue.position[2]=-2;
    blue.color[0]=0.01f;blue.color[1]=0.15f;blue.color[2]=1;blue.emission=3;
    s.objects.push_back(red);s.objects.push_back(blue);s.nextId=4;s.selected=0;
    s.camera.Frame({0,0,0},2.0f);
}
// 높은 삼각형 수와 화면 밖 객체를 함께 배치해 LOD/컬링 통계를 비교합니다.
void LoadGeometryDemo(Settings& s) {
    s=Settings{};s.objects.clear();s.floor=false;
    for(int z=0;z<8;++z) for(int x=0;x<8;++x) {
        SceneObject o;o.id=z*8+x+1;
        o.position[0]=(x-3.5f)*3;o.position[2]=-z*4.f;
        o.color[0]=0.25f+z*0.07f;s.objects.push_back(o);
    }
    s.nextId=65;s.selected=0;s.clusterLOD=true;s.clusterCulling=true;
    s.lodPixels=2.5f;s.researchView=3;s.camera.Frame({0,0,-5},2.0f);
}

void GenerateStressScene(Settings& s,int count,int mesh,int columns,float spacing,bool volume) {
    count=std::clamp(count,1,static_cast<int>(Settings::MaxObjects));
    columns=std::clamp(columns,1,64);spacing=std::clamp(spacing,2.5f,10.f);
    mesh=std::clamp(mesh,0,3);
    SceneObject material=s.Selected();
    s.objects.clear();s.objects.reserve(count);
    const int rows=volume?std::min(columns,(count+columns-1)/columns):(count+columns-1)/columns;
    for(int i=0;i<count;++i) {
        SceneObject o=material;o.id=i+1;o.mesh=mesh==3?i%2:mesh;
        o.scale=o.mesh==2?0.25f:1.f;
        o.position[0]=(i%columns-(std::min(columns,count)-1)*0.5f)*spacing;
        o.position[1]=volume?(i/(columns*rows))*spacing:0;
        o.position[2]=((i/columns)%rows-(rows-1)*0.5f)*spacing;
        s.objects.push_back(o);
    }
    s.selected=0;s.nextId=count+1;s.floor=false;
    FrameCamera(s);
}

// 렌더링, 컬링, 복셀 GI에서 동일한 변환을 사용합니다.
std::array<float,9> SceneObject::RotationMatrix() const {
    float x=rotation[0]*0.01745329252f,y=rotation[1]*0.01745329252f,z=rotation[2]*0.01745329252f;
    float a=std::cos(x),b=std::sin(x),c=std::cos(y),d=std::sin(y),e=std::cos(z),f=std::sin(z);
    return {e*c,f*c,-d, e*d*b-f*a,f*d*b+e*a,c*b, e*d*a+f*b,f*d*a-e*b,c*a};
}
Vec3 SceneObject::ToWorld(Vec3 p) const {
    auto r=RotationMatrix();p={p.x*scale*axisScale[0],p.y*scale*axisScale[1],p.z*scale*axisScale[2]};
    return {r[0]*p.x+r[3]*p.y+r[6]*p.z+position[0],r[1]*p.x+r[4]*p.y+r[7]*p.z+position[1],r[2]*p.x+r[5]*p.y+r[8]*p.z+position[2]};
}
Vec3 SceneObject::ToLocal(Vec3 p) const {
    auto r=RotationMatrix();p={p.x-position[0],p.y-position[1],p.z-position[2]};
    return {(r[0]*p.x+r[1]*p.y+r[2]*p.z)/(scale*axisScale[0]),(r[3]*p.x+r[4]*p.y+r[5]*p.z)/(scale*axisScale[1]),(r[6]*p.x+r[7]*p.y+r[8]*p.z)/(scale*axisScale[2])};
}
Vec3 SceneObject::WorldNormal(Vec3 n) const {
    auto r=RotationMatrix();n={n.x/axisScale[0],n.y/axisScale[1],n.z/axisScale[2]};
    Vec3 v={r[0]*n.x+r[3]*n.y+r[6]*n.z,r[1]*n.x+r[4]*n.y+r[7]*n.z,r[2]*n.x+r[5]*n.y+r[8]*n.z};
    float length=std::sqrt(std::max(1e-12f,v.x*v.x+v.y*v.y+v.z*v.z));return {v.x/length,v.y/length,v.z/length};
}
float SceneObject::MaxScale() const {return scale*std::max({axisScale[0],axisScale[1],axisScale[2]});}
float SceneObject::MinScale() const {return scale*std::min({axisScale[0],axisScale[1],axisScale[2]});}
