// Animator.cpp
#include "Animator.h"

// �{�[�����ɑΉ����鍡�̍s���Ԃ�
std::vector<DirectX::XMMATRIX> Animator::GetCurrentPoseMatrices() {
    std::vector<DirectX::XMMATRIX> result;
    if (!currentClip) return result;

    for (const auto& bone : currentClip->boneKeyframes) {
        // ���̃t���[���ԍ����v�Z
        int frameCount = (int)bone.second.size();
        float frameF = (currentTime / currentClip->duration) * frameCount;
        int frameIdx = std::min((int)frameF, frameCount - 1);

        result.push_back(bone.second[frameIdx]);
    }
    return result;
}
