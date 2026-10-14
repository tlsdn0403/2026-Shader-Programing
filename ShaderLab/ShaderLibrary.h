// 셰이더 저장소: 기능별 .vs/.fs 쌍과 컴파일된 GPU 프로그램을 관리합니다.
#pragma once
#include <GL/glew.h>
#include <array>
#include <string>

// Owns GPU programs. Failed reloads preserve the entire last valid set.
class ShaderLibrary {
public:
    ShaderLibrary() = default;
    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;
    ~ShaderLibrary();
    bool Initialize(const std::string& shaderDirectory);
    bool Reload();
    bool ReloadFrom(const std::string& vertex, const std::string& fragment, int effect);
    void Shutdown();
    GLuint Program(int effect) const;
    static const char* Name(int effect);
    const std::string& Status() const { return status; }
private:
    std::array<GLuint, 5> programs{};
    std::string directory, status;
};
