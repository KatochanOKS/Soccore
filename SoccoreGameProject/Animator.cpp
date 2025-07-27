#include "Animator.h"
#include"windows.h"
#include <DirectXMath.h>
using namespace DirectX;
//--------------------------------------
// コンストラクタ
//--------------------------------------
Animator::Animator() {}

//--------------------------------------
// アニメーションデータ・ボーン名をセット（初期化時に呼ぶ）
//--------------------------------------
void Animator::SetAnimations(
    const std::unordered_map<std::string, std::vector<Keyframe>>& anims,
    const std::vector<std::string>& bones,
    const std::vector<DirectX::XMMATRIX>& bindPoseMatrices
) {
    animations = anims;
    boneNames = bones;
    bindPoses = bindPoseMatrices; // ★追加
    if (!anims.empty())
        currentAnim = anims.begin()->first;
    currentTime = 0.0;
    isPlaying = true;

    boneMatrices.clear();
    if (!boneNames.empty())
        boneMatrices.resize(boneNames.size(), DirectX::XMMatrixIdentity());
}


//--------------------------------------
// 再生アニメーションを切り替える（Walk, Jump等）
//--------------------------------------
void Animator::SetAnimation(const std::string& animName) {
    if (animations.find(animName) != animations.end()) {
        currentAnim = animName;
        currentTime = 0.0;
    }
}

//--------------------------------------
// 毎フレーム、現在のボーン行列を更新する
//--------------------------------------
void Animator::Update(float deltaTime) {
    if (!isPlaying || animations.count(currentAnim) == 0) return;
    const auto& frames = animations[currentAnim];
    if (frames.empty()) return;

    // 時間を進めてループ
    currentTime += deltaTime;
    double animLength = frames.back().time;
    if (currentTime > animLength)
        currentTime = fmod(currentTime, animLength);

    // 現在時刻に最も近いキーフレームを探す
    size_t frameIdx = 0;
    while (frameIdx + 1 < frames.size() && frames[frameIdx + 1].time <= currentTime)
        ++frameIdx;

    boneMatrices.clear();
    for (size_t i = 0; i < frames[frameIdx].pose.size(); ++i) {
        DirectX::XMMATRIX pose = frames[frameIdx].pose[i];
        DirectX::XMMATRIX invBind = XMMatrixInverse(nullptr, bindPoses[i]);
        boneMatrices.push_back(invBind * pose); // ✅ 正しい順序

    }
    if (bindPoses.size() != frames[frameIdx].pose.size()) {
        char msg[256];
        sprintf_s(msg, "[Error] ボーン数不一致！bind=%zu, pose=%zu\n",
            bindPoses.size(), frames[frameIdx].pose.size());
        OutputDebugStringA(msg);
    }
}

// Animator.cpp
std::vector<XMMATRIX> Animator::GetSkinnedPose(const std::vector<XMMATRIX>& bindPoses) const {
    return boneMatrices; // ✅ もうスキニング行列になってるのでこれでOK！
}




//--------------------------------------
// 現在のボーン行列（描画用）を返す
//--------------------------------------
const std::vector<DirectX::XMMATRIX>& Animator::GetCurrentPose() const {
    return boneMatrices;
}
