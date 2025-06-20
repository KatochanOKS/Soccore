#pragma once
#include "AnimationClip.h"
#include <vector>
#include <string>
#include <DirectXMath.h>

class Animator {
public:
    void SetAnimation(AnimationClip* clip);
    void Update(float deltaTime);
    std::vector<DirectX::XMMATRIX> GetCurrentPoseMatrices(const std::vector<std::string>& boneNames);

private:
    AnimationClip* m_clip = nullptr;
    double m_time = 0.0;
};
