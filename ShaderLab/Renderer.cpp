// 렌더링 순서: FBO 준비 → 셰이더 선택 → uniform 전달 → 메시/바닥 그리기.
#include "Renderer.h"
#include "ImageCapture.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr float pi = 3.14159265359f;
Vec3 Cross(Vec3 a, Vec3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
float Dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
Vec3 Unit(Vec3 v) { float d=std::sqrt(Dot(v,v)); return {v.x/d,v.y/d,v.z/d}; }
// 정점 하나는 위치 xyz + 법선 xyz의 연속된 6개 float로 저장합니다.
void Vertex(std::vector<float>& data,Vec3 p,Vec3 n) {
    data.insert(data.end(),{p.x,p.y,p.z,n.x,n.y,n.z});
}
// 경도와 위도로 단위 구 표면의 위치를 계산합니다. 단위 구에서는 위치가 곧 법선입니다.
Vec3 SpherePoint(int x,int y) {
    float a=2*pi*x/64.0f,b=pi*y/32.0f;
    return {std::sin(b)*std::cos(a),std::cos(b),std::sin(b)*std::sin(a)};
}
}

// VBO에는 정점 데이터, VAO에는 셰이더 입력 위치와 데이터 배치 정보를 저장합니다.
Renderer::Mesh Renderer::Upload(const std::vector<float>& data) {
    Mesh m; m.count=static_cast<GLsizei>(data.size()/6);
    glGenVertexArrays(1,&m.vao); glGenBuffers(1,&m.vbo);
    glBindVertexArray(m.vao); glBindBuffer(GL_ARRAY_BUFFER,m.vbo);
    glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(float),data.data(),GL_STATIC_DRAW);
    for(GLuint i=0;i<2;++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i,3,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(i*3*sizeof(float)));
    }
    glBindVertexArray(0); m.clustered.Build(data); return m;
}
// 외부 모델 대신 구·큐브·세분화 평면을 생성하고 GPU로 업로드합니다.
bool Renderer::Initialize(const std::string& shaderDirectory) {
    if(!shaders.Initialize(shaderDirectory)) return false;
    std::vector<float> data;
    for(int y=0;y<32;++y) for(int x=0;x<64;++x) {
        Vec3 a=SpherePoint(x,y),b=SpherePoint(x+1,y),c=SpherePoint(x+1,y+1),d=SpherePoint(x,y+1);
        for(Vec3 p:{a,b,c,a,c,d}) Vertex(data,p,p);
    }
    sphere=Upload(data); data.clear();
    const Vec3 normals[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for(Vec3 n:normals) {
        Vec3 u=std::abs(n.y)>0.5f?Vec3{1,0,0}:Unit(Cross({0,1,0},n));
        Vec3 v=Cross(n,u);
        const int corners[6][2]={{-1,-1},{1,-1},{1,1},{-1,-1},{1,1},{-1,1}};
        for(auto& c:corners) Vertex(data,{(n.x+u.x*c[0]+v.x*c[1])*0.8f,(n.y+u.y*c[0]+v.y*c[1])*0.8f,(n.z+u.z*c[0]+v.z*c[1])*0.8f},n);
    }
    // 평면은 물결 변형에 충분한 정점이 있도록 잘게 나눕니다. 기준 바닥으로도 재사용합니다.
    cube=Upload(data); data.clear();
    for(int z=0;z<64;++z) for(int x=0;x<64;++x) {
        const int corners[6][2]={{0,0},{0,1},{1,1},{0,0},{1,1},{1,0}};
        for(auto& c:corners) Vertex(data,{(x+c[0])/64.0f*8.0f-4.0f,-1.2f,(z+c[1])/64.0f*8.0f-4.0f},{0,1,0});
    }
    plane=Upload(data);
    glGenQueries(2,timerQueries);
    glGenFramebuffers(1,&framebuffer); glGenTextures(1,&colorTexture); glGenRenderbuffers(1,&depthBuffer);
    return true;
}
// 미리보기 크기가 바뀔 때만 컬러 텍스처와 깊이 버퍼의 저장 공간을 다시 할당합니다.
void Renderer::Resize(int w,int h) {
    if(w==width && h==height) return;
    width=w; height=h;
    glBindTexture(GL_TEXTURE_2D,colorTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glBindRenderbuffer(GL_RENDERBUFFER,depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,w,h);
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,colorTexture,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depthBuffer);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Preview framebuffer is incomplete");
}
// VAO가 지정한 정점 배열을 삼각형 단위로 그립니다.
void Renderer::Draw(const Mesh& mesh) { glBindVertexArray(mesh.vao); glDrawArrays(GL_TRIANGLES,0,mesh.count); }


// 화면에 직접 그리지 않고 FBO에 그립니다. 완성된 컬러 텍스처를 UI가 표시합니다.
void Renderer::Render(int w,int h,const Settings& s,float time) {
    Resize(std::clamp(w,1,4096),std::clamp(h,1,4096));
    gi.Update(s);
    stats={};
    // GPU 시간은 비동기 쿼리로 읽습니다. CPU 복셀 재계산 시간은 별도 표시합니다.
    for(int i=0;i<2;++i) if(timerPending[i]) {
        GLint ready=0;glGetQueryObjectiv(timerQueries[i],GL_QUERY_RESULT_AVAILABLE,&ready);
        if(ready) {GLuint64 ns=0;glGetQueryObjectui64v(timerQueries[i],GL_QUERY_RESULT,&ns);gpuMs=ns/1e6;timerPending[i]=false;}
    }
    bool timed=!timerPending[timerIndex];
    if(timed) glBeginQuery(GL_TIME_ELAPSED,timerQueries[timerIndex]);
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer); glViewport(0,0,width,height);
    glEnable(GL_DEPTH_TEST); glDisable(GL_BLEND); glDisable(GL_CULL_FACE);
    glClearColor(0.025f,0.036f,0.052f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    Vec3 eye = s.camera.Position();
    Matrix4 vp = s.camera.ViewProjection(static_cast<float>(width) / height);
    // 같은 구/큐브/평면 GPU 버퍼를 재사용하고 객체마다 셰이더와 uniform만 바꿉니다.
    auto drawObject = [&](const SceneObject& object, bool referenceFloor) {
    GLuint program=shaders.Program(object.effect);
    glUseProgram(program);
    gi.Bind(program,s);
    glUniform1f(glGetUniformLocation(program,"uEmission"),referenceFloor?0.f:object.emission);
    // C++에서 셰이더로 전달하는 값입니다.
    auto loc=[&](const char* name){return glGetUniformLocation(program,name);};
    glUniformMatrix4fv(loc("uViewProjection"),1,GL_FALSE,vp.data());
    glUniform3f(loc("uCamera"),eye.x,eye.y,eye.z);
    glUniform3fv(loc("uLight"),1,s.light.position);
    glUniform3fv(loc("uLightColor"),1,s.light.color);
    Vec3 direction=s.light.Direction();
    glUniform3f(loc("uLightDirection"),direction.x,direction.y,direction.z);
    glUniform1i(loc("uLightType"),s.light.type);
    glUniform1f(loc("uLightRange"),std::max(s.light.range,0.1f));
    float inner=std::clamp(s.light.innerAngle,0.f,88.f);
    float outer=std::clamp(s.light.outerAngle,inner+0.1f,89.f);
    glUniform1f(loc("uSpotInnerCos"),std::cos(inner*pi/180));
    glUniform1f(loc("uSpotOuterCos"),std::cos(outer*pi/180));
    glUniform3fv(loc("uBaseColor"),1,object.color);
    glUniform1f(loc("uTime"),time); glUniform1f(loc("uWave"),object.wave);
    glUniform1f(loc("uRoughness"),object.roughness); glUniform1f(loc("uRim"),object.rim);
    glUniform1f(loc("uDissolve"),object.dissolve); glUniform1f(loc("uPower"),s.light.enabled?s.light.intensity:0.f); glUniform1f(loc("uAmbient"),s.ambient);
    glUniform3fv(loc("uPosition"),1,object.position);
    glUniform1f(loc("uScale"),object.scale);
    glUniform1i(loc("uFloor"),referenceFloor?1:0);

    glPolygonMode(GL_FRONT_AND_BACK,(!referenceFloor && s.wireframe)?GL_LINE:GL_FILL);
    const Mesh& mesh=object.mesh==0?sphere:object.mesh==1?cube:plane;
    if(!referenceFloor && (s.clusterLOD || s.clusterCulling || s.researchView>=2))
        mesh.clustered.Draw(program,object,s,vp,height,stats);
    else {
        Draw(mesh);
        if(!referenceFloor) {stats.fullTriangles+=mesh.count/3;stats.submittedTriangles+=mesh.count/3;++stats.drawCalls;++stats.lodObjects[0];}
    }
    };
    for(const auto& object:s.objects) drawObject(object,false);
    // 기준 바닥은 장면 전체에 한 번만 그립니다.
    if(s.floor) {
        SceneObject reference; reference.mesh=2;
        reference.position[1]=-0.02f;
        drawObject(reference,true);
    }
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glBindVertexArray(0); glUseProgram(0); glBindFramebuffer(GL_FRAMEBUFFER,0);
    if(timed) {glEndQuery(GL_TIME_ELAPSED);timerPending[timerIndex]=true;timerIndex=1-timerIndex;}
}
int Renderer::Triangles(int mesh) const { return (mesh==0?sphere:mesh==1?cube:plane).count/3; }
// 테스트용 화면 해시입니다. 같은 렌더링 조건에서 출력이 유지되는지 비교합니다.
unsigned long long Renderer::Fingerprint() const {
    std::vector<unsigned char> pixels(width*height*4);
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer); glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data()); glBindFramebuffer(GL_FRAMEBUFFER,0);
    unsigned long long result=1469598103934665603ull;
    for(unsigned char v:pixels) {result^=v; result*=1099511628211ull;}
    return result;
}

// UI를 제외한 미리보기만 저장하기 위해 해당 FBO를 선택합니다.
bool Renderer::Capture(const std::string& path) const {
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
    bool saved=SaveFramebufferPng(path,width,height);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    return saved;
}
// Application이 컨텍스트를 파괴하기 전에 호출합니다. 해제한 핸들은 0으로 초기화합니다.
void Renderer::Shutdown() {
    for(Mesh* m:{&sphere,&cube,&plane}) {m->clustered.Shutdown();glDeleteBuffers(1,&m->vbo);glDeleteVertexArrays(1,&m->vao);*m={};}
    gi.Shutdown();
    glDeleteQueries(2,timerQueries);timerQueries[0]=timerQueries[1]=0;
    timerPending[0]=timerPending[1]=false;
    shaders.Shutdown();
    glDeleteFramebuffers(1,&framebuffer); glDeleteTextures(1,&colorTexture); glDeleteRenderbuffers(1,&depthBuffer);
    framebuffer=colorTexture=depthBuffer=0; width=height=0;
}
