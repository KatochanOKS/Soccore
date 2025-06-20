#include "Animator.h"
using namespace DirectX;

void Animator::SetAnimation(AnimationClip* clip) {
    m_clip = clip;
    m_time = 0.0;
}
void Animator::Update(float deltaTime) {
    if (!m_clip) return;
    m_time += deltaTime;
    if (m_time > m_clip->duration)
        m_time = 0.0; // ループ
}
std::vector<XMMATRIX> Animator::GetCurrentPoseMatrices(const std::vector<std::string>& boneNames) {
    std::vector<XMMATRIX> result(boneNames.size(), XMMatrixIdentity());
    if (!m_clip) return result;
    for (size_t i = 0; i < boneNames.size(); ++i) {
        for (const BoneAnim& ba : m_clip->boneAnims) {
            if (ba.name == boneNames[i] && !ba.keyframes.empty()) {
                // 一番近いキーフレーム
                int idx = (int)(m_time * 30.0f); // 30fps仮定
                if (idx >= (int)ba.keyframes.size()) idx = (int)ba.keyframes.size() - 1;
                result[i] = ba.keyframes[idx].transform;
                break;
            }
        }
    }
    return result;
}
