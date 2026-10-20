#include "ClusterMesh.h"
#include <meshoptimizer.h>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace {
// 클립 공간의 6개 평면으로 경계 구를 검사하고 물결 변위만큼 반경을 확장합니다.
bool Visible(const Matrix4& m,Vec3 c,float r) {
    for(int axis=0;axis<3;++axis) for(int sign:{-1,1}) {
        float a=m[3]+sign*m[axis], b=m[7]+sign*m[4+axis];
        float d=m[15]+sign*m[12+axis], z=m[11]+sign*m[8+axis];
        if(a*c.x+b*c.y+z*c.z+d < -r*std::sqrt(a*a+b*b+z*z)) return false;
    }
    return true;
}
}
void ClusterMesh::Build(const std::vector<float>& input) {
    Shutdown();
    const size_t count=input.size()/6;
    std::vector<unsigned> remap(count);
    size_t unique=meshopt_generateVertexRemap(remap.data(),nullptr,count,input.data(),count,6*sizeof(float));
    std::vector<float> vertices(unique*6);
    std::vector<unsigned> original(count);
    meshopt_remapVertexBuffer(vertices.data(),input.data(),count,6*sizeof(float),remap.data());
    meshopt_remapIndexBuffer(original.data(),nullptr,count,remap.data());
    float extent=meshopt_simplifyScale(vertices.data(),unique,6*sizeof(float));
    for(int level=0;level<4;++level) {
        std::vector<unsigned> indices=original;
        float error=0;
        if(level>0) {
            size_t target=std::max<size_t>(12,(count>>(level*2))/3*3);
            size_t reduced=meshopt_simplify(indices.data(),original.data(),count,vertices.data(),unique,
                6*sizeof(float),target,0.2f,0,&error);
            indices.resize(reduced);
        }
        // 객체 전체 LOD 선택 후 meshlet 단위로 컬링합니다. 인접 클러스터는 같은 LOD를 사용합니다.
        Level result; result.error=error*extent;
        const size_t bound=meshopt_buildMeshletsBound(indices.size(),64,124);
        std::vector<meshopt_Meshlet> meshlets(bound);
        std::vector<unsigned> meshletVertices(bound*64);
        std::vector<unsigned char> triangles(bound*124*3);
        size_t clusters=meshopt_buildMeshlets(meshlets.data(),meshletVertices.data(),triangles.data(),
            indices.data(),indices.size(),vertices.data(),unique,6*sizeof(float),64,124,0.f);
        std::vector<unsigned> flattened;
        for(size_t i=0;i<clusters;++i) {
            const auto& cluster=meshlets[i];
            auto bounds=meshopt_computeMeshletBounds(meshletVertices.data()+cluster.vertex_offset,
                triangles.data()+cluster.triangle_offset,cluster.triangle_count,vertices.data(),unique,6*sizeof(float));
            result.clusters.push_back({static_cast<unsigned>(flattened.size()),cluster.triangle_count*3,
                {bounds.center[0],bounds.center[1],bounds.center[2]},bounds.radius});
            for(unsigned t=0;t<cluster.triangle_count*3;++t)
                flattened.push_back(meshletVertices[cluster.vertex_offset+triangles[cluster.triangle_offset+t]]);
        }
        result.indices=static_cast<unsigned>(flattened.size());
        glGenVertexArrays(1,&result.vao);glGenBuffers(1,&result.vbo);glGenBuffers(1,&result.ebo);
        glBindVertexArray(result.vao);glBindBuffer(GL_ARRAY_BUFFER,result.vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
        for(GLuint a=0;a<2;++a) {
            glEnableVertexAttribArray(a);
            glVertexAttribPointer(a,3,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(a*3*sizeof(float)));
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,result.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,flattened.size()*sizeof(unsigned),flattened.data(),GL_STATIC_DRAW);
        levels.push_back(result);
    }
    glBindVertexArray(0);
}
void ClusterMesh::Draw(GLuint program,const SceneObject& o,const Settings& s,const Matrix4& vp,
                       int height,GeometryStats& stats) const {
    if(levels.empty()) return;
    unsigned lod=0;
    // clip.w는 카메라 전방 깊이입니다. 화면 가장자리에서도 거리 대신 깊이를 사용합니다.
    float viewDepth=vp[3]*o.position[0]+vp[7]*o.position[1]+vp[11]*o.position[2]+vp[15];
    float depth=std::max(0.1f,viewDepth-(o.mesh==2?6.f:1.75f)*o.scale);
    // 물결의 변위 오차는 단순화 오차에 없으므로 원본 LOD를 유지합니다.
    if(s.clusterLOD && o.effect!=4)
        for(unsigned i=1;i<levels.size();++i)
            if(levels[i].error*o.scale*height/(0.828427f*depth)<=s.lodPixels) lod=i;
    const auto& level=levels[lod];
    stats.fullTriangles+=levels[0].indices/3; ++stats.lodObjects[lod];
    glBindVertexArray(level.vao);
    const float palette[4][3]={{0.18f,0.8f,0.4f},{0.15f,0.5f,1.f},{1.f,0.65f,0.1f},{0.95f,0.15f,0.35f}};
    GLint color=glGetUniformLocation(program,"uDebugColor");
    for(unsigned i=0;i<level.clusters.size();++i) {
        const auto& c=level.clusters[i]; ++stats.clusters;
        Vec3 center={c.center.x*o.scale+o.position[0],c.center.y*o.scale+o.position[1],c.center.z*o.scale+o.position[2]};
        float radius=(c.radius+(o.effect==4?o.wave:0))*o.scale;
        if(s.clusterCulling && !Visible(vp,center,radius)) {++stats.culled;continue;}
        if(s.researchView==3) glUniform3fv(color,1,palette[lod]);
        else glUniform3f(color,0.2f+0.8f*std::fmod(i*0.618f,1.f),0.2f+0.8f*std::fmod(i*0.317f,1.f),0.2f+0.8f*std::fmod(i*0.713f,1.f));
        glDrawElements(GL_TRIANGLES,c.count,GL_UNSIGNED_INT,reinterpret_cast<void*>(static_cast<size_t>(c.first)*sizeof(unsigned)));
        stats.submittedTriangles+=c.count/3;++stats.drawCalls;
    }
}
unsigned ClusterMesh::Triangles() const {return levels.empty()?0:levels[0].indices/3;}
void ClusterMesh::Shutdown() {
    for(auto& level:levels) {
        glDeleteBuffers(1,&level.vbo);glDeleteBuffers(1,&level.ebo);glDeleteVertexArrays(1,&level.vao);
    }
    levels.clear();
}
