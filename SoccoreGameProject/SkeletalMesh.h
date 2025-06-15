#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>

struct Bone {
    std::string name;
    int parentIndex;
    DirectX::XMMATRIX offsetMatrix;
};
struct SkinVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    int boneIndices[4] = { 0,0,0,0 };
    float boneWeights[4] = { 0,0,0,0 };
};
class SkeletalMesh {
public:
    std::vector<SkinVertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<Bone> bones;
    // ...バッファなど後で追加
};
