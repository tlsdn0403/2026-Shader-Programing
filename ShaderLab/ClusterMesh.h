// meshoptimizer로 단순화 LOD와 meshlet을 만들고 CPU에서 보수적인 프러스텀 컬링을 합니다.
#pragma once
#include <GL/glew.h>
#include "SceneSettings.h"
#include <vector>

struct GeometryStats {
    unsigned fullTriangles=0, submittedTriangles=0, clusters=0, culled=0, drawCalls=0;
    unsigned lodObjects[4]{};
};
class ClusterMesh {
public:
    void Build(const std::vector<float>& vertices);
    void Draw(GLuint program,const SceneObject& object,const Settings& settings,
              const Matrix4& viewProjection,int viewportHeight,GeometryStats& stats) const;
    void Shutdown();
    unsigned Triangles() const;
private:
    struct Cluster { unsigned first=0,count=0; Vec3 center{}; float radius=0; };
    struct Level { GLuint vao=0,vbo=0,ebo=0; unsigned indices=0; float error=0; std::vector<Cluster> clusters; };
    std::vector<Level> levels;
};
