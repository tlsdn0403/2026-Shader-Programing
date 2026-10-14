// 현재 바인딩된 프레임버퍼를 PNG로 저장하는 상태 없는 유틸리티입니다.
#pragma once
#include <string>

// Saves the currently bound framebuffer as a vertically corrected PNG.
bool SaveFramebufferPng(const std::string& path, int width, int height);
