#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>

struct BoneKeyframe {
    double time;
    DirectX::XMMATRIX transform; // 各ボーンのグローバル変換
};

struct BoneAnim {
    std::string name;
    std::vector<BoneKeyframe> keyframes;
};

class AnimationClip {
public:
    double duration = 0.0;
    std::vector<BoneAnim> boneAnims;
};