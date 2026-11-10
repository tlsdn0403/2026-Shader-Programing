#include "TextureLibrary.h"
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <wincodec.h>
#include <wrl/client.h>
#endif

std::string ChooseTextureFile() {
#ifdef _WIN32
    wchar_t path[32768]{};
    OPENFILENAMEW dialog{}; dialog.lStructSize=sizeof(dialog);
    dialog.hwndOwner=GetActiveWindow(); dialog.lpstrFile=path; dialog.nMaxFile=32768;
    dialog.lpstrTitle=L"Select a base color texture";
    dialog.lpstrFilter=L"Images (PNG, JPEG, BMP)\0*.png;*.jpg;*.jpeg;*.bmp\0All files\0*.*\0";
    dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_EXPLORER;
    if(GetOpenFileNameW(&dialog)) return std::filesystem::path(path).u8string();
#endif
    return {};
}
const TextureLibrary::Image* TextureLibrary::Find(const std::string& path) const {
    auto it=images.find(path); return it==images.end()?nullptr:&it->second;
}
bool TextureLibrary::Load(const std::string& path,std::string& error) {
    error.clear(); if(Find(path)) return true;
#ifdef _WIN32
    // WIC를 사용하므로 JPG/PNG/BMP 디코더를 별도로 배포할 필요가 없습니다.
    HRESULT initialized=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    struct ComScope { bool owned; ~ComScope(){if(owned) CoUninitialize();} } scope{SUCCEEDED(initialized)};
    GLuint pending=0;
    try {
        if(FAILED(initialized) && initialized!=RPC_E_CHANGED_MODE) throw std::runtime_error("Cannot initialize image decoder.");
        auto check=[](HRESULT hr){if(FAILED(hr)) throw std::runtime_error("Cannot read image. Choose a valid PNG, JPEG or BMP file.");};
        using Microsoft::WRL::ComPtr;
        ComPtr<IWICImagingFactory> factory; ComPtr<IWICBitmapDecoder> decoder;
        ComPtr<IWICBitmapFrameDecode> frame; ComPtr<IWICFormatConverter> converter;
        check(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(factory.GetAddressOf())));
        auto wide=std::filesystem::u8path(path).wstring();
        check(factory->CreateDecoderFromFilename(wide.c_str(),nullptr,GENERIC_READ,WICDecodeMetadataCacheOnDemand,&decoder));
        check(decoder->GetFrame(0,&frame)); unsigned w=0,h=0; check(frame->GetSize(&w,&h));
        GLint maxSize=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);
        if(!w||!h||w>static_cast<unsigned>(maxSize)||h>static_cast<unsigned>(maxSize)||static_cast<unsigned long long>(w)*h>16777216)
            throw std::runtime_error("Image too large. Maximum 16 megapixels and GPU texture size limit.");
        check(factory->CreateFormatConverter(&converter));
        check(converter->Initialize(frame.Get(),GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom));
        std::vector<unsigned char> pixels(static_cast<size_t>(w)*h*4);
        check(converter->CopyPixels(nullptr,w*4,static_cast<UINT>(pixels.size()),pixels.data()));
        // WIC의 위쪽 원점 이미지를 그대로 보내고 GLSL의 V 좌표를 반전합니다.
        glActiveTexture(GL_TEXTURE0);GLint previous=0;glGetIntegerv(GL_TEXTURE_BINDING_2D,&previous);
        glGenTextures(1,&pending);glBindTexture(GL_TEXTURE_2D,pending);
        glTexImage2D(GL_TEXTURE_2D,0,GL_SRGB8_ALPHA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D); GLenum result=glGetError();glBindTexture(GL_TEXTURE_2D,previous);
        if(result!=GL_NO_ERROR) throw std::runtime_error("GPU texture upload failed.");
        images.emplace(path,Image{pending,w,h});return true;
    } catch(const std::exception& e) { if(pending) glDeleteTextures(1,&pending);error=e.what();return false; }
#else
    error="Image import currently requires Windows.";return false;
#endif
}
void TextureLibrary::Collect(const Settings& s) {
    std::unordered_set<std::string> used;for(const auto& o:s.objects) used.insert(o.texturePath);
    for(auto it=images.begin();it!=images.end();) {
        if(!used.count(it->first)){glDeleteTextures(1,&it->second.id);it=images.erase(it);}else ++it;
    }
}
void TextureLibrary::Shutdown(){for(auto& pair:images)glDeleteTextures(1,&pair.second.id);images.clear();}
