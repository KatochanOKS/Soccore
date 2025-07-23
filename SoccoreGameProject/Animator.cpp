#include "Animator.h"

//--------------------------------------
// コンストラクタ
//--------------------------------------
Animator::Animator() {}

//--------------------------------------
// アニメーションデータ・ボーン名をセット（初期化時に呼ぶ）
//--------------------------------------
void Animator::SetAnimations(const std::unordered_map<std::string, std::vector<Keyframe>>& anims, const std::vector<std::string>& bones) {
    animations = anims;
    boneNames = bones;
    if (!anims.empty())
        currentAnim = anims.begin()->first; // 最初のアニメにセット
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

    // キーフレームからボーン行列セット
    boneMatrices = frames[frameIdx].pose;
}

//--------------------------------------
// 現在のボーン行列（描画用）を返す
//--------------------------------------
const std::vector<DirectX::XMMATRIX>& Animator::GetCurrentPose() const {
    return boneMatrices;
}
