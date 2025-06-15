// Animator.cpp
#include "Animator.h"

// ボーン名に対応する今の行列を返す
std::vector<DirectX::XMMATRIX> Animator::GetCurrentPoseMatrices() {
    std::vector<DirectX::XMMATRIX> result;
    if (!currentClip) return result;

    for (const auto& bone : currentClip->boneKeyframes) {
        // 今のフレーム番号を計算
        int frameCount = (int)bone.second.size();
        float frameF = (currentTime / currentClip->duration) * frameCount;
        int frameIdx = std::min((int)frameF, frameCount - 1);

        result.push_back(bone.second[frameIdx]);
    }
    return result;
}
