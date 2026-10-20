// 파일 읽기 → 각 단계 컴파일 → 프로그램 링크. 재로딩 실패 시 기존 프로그램을 보존합니다.
#include "ShaderLibrary.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace {
// GLSL 파일 전체를 읽습니다. 파일이 없으면 경로가 포함된 오류를 전달합니다.
std::string Read(const std::string& path) {
    std::ifstream f(path);
    if(!f) throw std::runtime_error("Cannot read: "+path);
    return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};
}
// 확장자가 아니라 GL_VERTEX_SHADER / GL_FRAGMENT_SHADER 인수로 셰이더 종류가 결정됩니다.
GLuint Compile(GLenum type,const std::string& source) {
    GLuint shader=glCreateShader(type);
    const char* text=source.c_str(); glShaderSource(shader,1,&text,nullptr); glCompileShader(shader);
    GLint ok=0; glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(!ok) {
        char log[8192]{}; glGetShaderInfoLog(shader,sizeof(log),nullptr,log); glDeleteShader(shader);
        throw std::runtime_error(log);
    }
    return shader;
}
}
namespace {
// 버텍스와 프래그먼트 셰이더를 컴파일한 뒤 링크해 하나의 GPU 프로그램을 만듭니다.
GLuint BuildProgram(const std::string& vertex,const std::string& fragment) {
    GLuint vs=0,fs=0,next=0;
    try {
        try { vs=Compile(GL_VERTEX_SHADER,vertex); }
        catch(const std::exception& e) { throw std::runtime_error(std::string("Vertex shader: ")+e.what()); }
        try { fs=Compile(GL_FRAGMENT_SHADER,fragment); }
        catch(const std::exception& e) { throw std::runtime_error(std::string("Fragment shader: ")+e.what()); }
        next=glCreateProgram(); glAttachShader(next,vs); glAttachShader(next,fs); glLinkProgram(next);
        GLint ok=0; glGetProgramiv(next,GL_LINK_STATUS,&ok);
        if(!ok) { char log[8192]{}; glGetProgramInfoLog(next,sizeof(log),nullptr,log); throw std::runtime_error(log); }
        glDeleteShader(vs); glDeleteShader(fs);
        return next;
    } catch(...) {
        if(vs) glDeleteShader(vs); if(fs) glDeleteShader(fs); if(next) glDeleteProgram(next);
        throw;
    }
}
}
// UI의 효과 번호와 실제 파일 이름을 연결합니다: Lighting.vs + Lighting.fs 등.
const char* ShaderLibrary::Name(int effect) {
    static const char* names[]={"Lighting","Toon","Fresnel","Dissolve","Wave","Phong"};
    return names[std::clamp(effect,0,5)];
}
// 모든 프로그램을 임시로 생성합니다. 전부 성공한 경우에만 기존 세트를 교체합니다.
bool ShaderLibrary::Reload() {
    std::array<GLuint,6> next{};
    int effect=0;
    try {
        for(;effect<6;++effect) {
            std::string base=directory+"/"+Name(effect);
            std::string fragment=Read(base+".fs");
            for(const char* shared:{"Light.glsl","Research.glsl"}) {
                const std::string include=std::string("#include \"")+shared+"\"";
                auto at=fragment.find(include);
                if(at!=std::string::npos) fragment.replace(at,include.size(),Read(directory+"/"+shared));
            }
            next[effect]=BuildProgram(Read(base+".vs"),fragment);
        }
        // Commit only after every pair links, so a failed reload preserves all effects.
        for(GLuint old:programs) glDeleteProgram(old);
        programs=next;
        status="All 6 shader pairs ready. Reload preserves all programs on errors.";
        return true;
    } catch(const std::exception& e) {
        for(GLuint pending:next) glDeleteProgram(pending);
        status=std::string(Name(effect))+": "+e.what();
        return false;
    }
}
// 문자열로 전달된 셰이더 한 쌍을 재컴파일합니다. 주로 오류 복구 테스트에 사용합니다.
bool ShaderLibrary::ReloadFrom(const std::string& vertex,const std::string& fragment,int effect) {
    if(effect<0 || effect>=6) {status="Invalid effect index";return false;}
    try {
        GLuint next=BuildProgram(vertex,fragment);
        glDeleteProgram(programs[effect]); programs[effect]=next;
        status=std::string(Name(effect))+": ready.";
        return true;
    } catch(const std::exception& e) {
        status=std::string(Name(effect))+": "+e.what(); return false;
    }
}

ShaderLibrary::~ShaderLibrary() { Shutdown(); }
bool ShaderLibrary::Initialize(const std::string& shaderDirectory) {
    directory = shaderDirectory;
    return Reload();
}
GLuint ShaderLibrary::Program(int effect) const {
    return programs[std::clamp(effect, 0, 5)];
}
// 프로그램은 컨텍스트가 살아 있을 때 삭제해야 합니다. 중복 호출해도 삭제하지 않도록 0으로 만듭니다.
void ShaderLibrary::Shutdown() {
    for (GLuint program : programs) if (program) glDeleteProgram(program);
    programs.fill(0);
}
