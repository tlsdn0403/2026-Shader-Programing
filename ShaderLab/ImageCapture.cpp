// GPU 픽셀 읽기 → 이미지 상하 반전 → LodePNG로 파일 저장.
#include <GL/glew.h>
#include "ImageCapture.h"
#include "../SimpleGame/LoadPng.h"
#include <vector>
#include <algorithm>

bool SaveFramebufferPng(const std::string& path,int width,int height) {
    if(width<=0 || height<=0) return false;
    std::vector<unsigned char> pixels(width*height*4);
    // OpenGL 이미지의 원점은 좌측 하단이므로 저장 전 행 순서를 뒤집습니다.
    glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    for(int y=0;y<height/2;++y)
        std::swap_ranges(pixels.begin()+y*width*4,pixels.begin()+(y+1)*width*4,pixels.begin()+(height-1-y)*width*4);
    return lodepng::encode(path,pixels,static_cast<unsigned>(width),static_cast<unsigned>(height))==0;
}
