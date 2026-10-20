// 장면 렌더러: 메시와 화면 밖 렌더 타깃(FBO)을 관리하고 결과를 텍스처로 제공합니다.
#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include "SceneSettings.h"
#include "ShaderLibrary.h"
#include "VoxelGI.h"
#include "ClusterMesh.h"


class Renderer {
public:
    bool Initialize(const std::string& shaderDirectory);
    bool Reload() { return shaders.Reload(); }
    bool ReloadFrom(const std::string& vertex, const std::string& fragment, int effect = 0) { return shaders.ReloadFrom(vertex, fragment, effect); }
    static const char* ShaderName(int effect) { return ShaderLibrary::Name(effect); }
    void Render(int width, int height, const Settings& settings, float time);
    void Shutdown();
    GLuint Texture() const { return colorTexture; }
    const std::string& Status() const { return shaders.Status(); }
    int Triangles(int mesh) const;
    bool Capture(const std::string& path) const;
    unsigned long long Fingerprint() const;
    const GeometryStats& Statistics() const { return stats; }
    const VoxelGI& GI() const { return gi; }
    double GpuMs() const { return gpuMs; }
private:
    VoxelGI gi;
    GeometryStats stats;
    GLuint timerQueries[2]{};
    bool timerPending[2]{};
    int timerIndex=0;
    double gpuMs=0;
    struct Mesh { GLuint vao = 0, vbo = 0; GLsizei count = 0; ClusterMesh clustered; };
    Mesh sphere, cube, plane;
    Mesh Upload(const std::vector<float>& vertices);
    void Resize(int width, int height);
    void Draw(const Mesh& mesh);
    ShaderLibrary shaders;
    GLuint framebuffer = 0, colorTexture = 0, depthBuffer = 0;
    int width = 0, height = 0;
};
