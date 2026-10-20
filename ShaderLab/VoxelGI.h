// 장면의 단순 도형을 CPU에서 복셀화하고 방사휘도/불투명도 3D 텍스처를 만듭니다.
#pragma once
#include <GL/glew.h>
#include "SceneSettings.h"
#include <vector>

class VoxelGI {
public:
    static constexpr int Resolution = 64;
    static constexpr float Extent = 16.0f;
    static constexpr float CellSize = Extent / Resolution;
    bool Update(const Settings& settings);
    void Bind(GLuint program, const Settings& settings) const;
    void Shutdown();
    int Occupied() const { return occupied; }
    int Skipped() const { return skipped; }
    double UpdateMs() const { return updateMs; }
    unsigned Updates() const { return updates; }
    static Vec3 Minimum() { return {-8,-6,-8}; }
private:
    GLuint texture = 0;
    std::vector<float> previousKey;
    int occupied = 0, skipped = 0;
    double updateMs = 0;
    unsigned updates = 0;
};
