#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include "SceneSettings.h"

// 파일 경로별 GPU 텍스처를 공유합니다. 컨텍스트가 살아 있을 때 Shutdown을 호출합니다.
class TextureLibrary {
public:
    struct Image { GLuint id=0; unsigned width=0, height=0; };
    bool Load(const std::string& utf8Path, std::string& error);
    const Image* Find(const std::string& path) const;
    void Collect(const Settings& settings);
    void Shutdown();
private:
    std::unordered_map<std::string,Image> images;
};

// 취소하면 빈 문자열을 반환합니다. Windows 파일 창은 유니코드 경로를 사용합니다.
std::string ChooseTextureFile();
