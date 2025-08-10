#include "Animator.h"
#include "windows.h"
#include <DirectXMath.h>
using namespace DirectX;

//--------------------------------------
// コンストラクタ（初期化）
//--------------------------------------
Animator::Animator() {
    // 必要ならここで初期値セット
}

//--------------------------------------
// アニメーションとボーン名・バインドポーズ配列をセット
// 初期化時やFBX読込時に一度だけ呼ぶ
//--------------------------------------
void Animator::SetAnimations(
    const std::unordered_map<std::string, std::vector<Keyframe>>& anims,     // アニメ名→キーフレームリスト
    const std::vector<std::string>& bones,                                   // ボーン名一覧（順序はFBXと一致）
    const std::vector<DirectX::XMMATRIX>& bindPoseMatrices                   // 各ボーンのバインドポーズ行列
) {
    animations = anims;        // 全アニメを保存
    boneNames = bones;         // ボーン名を保存
    bindPoses = bindPoseMatrices; // バインドポーズ（静止ポーズの変換行列。後述）

    // 最初に登録されたアニメを再生中に
    if (!anims.empty())
        currentAnim = anims.begin()->first;
    currentTime = 0.0;
    isPlaying = true;

    // ボーン数分の行列配列を初期化（XMMatrixIdentityで全部単位行列）
    boneMatrices.clear();
    if (!boneNames.empty())
        boneMatrices.resize(boneNames.size(), DirectX::XMMatrixIdentity());
}

//--------------------------------------
// アニメーションを切り替える
//（例：Walk→Idle, Idle→Jumpなど）
//--------------------------------------
void Animator::SetAnimation(const std::string& animName) {
    // その名前のアニメが存在すれば切り替え
    if (animations.find(animName) != animations.end()) {
        currentAnim = animName;
        currentTime = 0.0; // 頭から再生
    }
}

//--------------------------------------
// 毎フレーム「現在の時刻」に合わせて各ボーン行列を計算する
//--------------------------------------
void Animator::Update(float deltaTime) {
    // 再生中フラグがOFF or アニメ無しなら何もしない
    if (!isPlaying || animations.count(currentAnim) == 0) return;
    const auto& frames = animations[currentAnim];  // 今再生してるアニメの全キーフレーム
    if (frames.empty()) return;

    // 再生時刻を進める（ループ再生：アニメ長を超えたら巻き戻す）
    currentTime += deltaTime;
    double animLength = frames.back().time;
    if (currentTime > animLength)
        currentTime = fmod(currentTime, animLength);

    // 今の再生時刻に一番近いキーフレーム番号を探す（リニアサーチでOK：アニメは数十フレーム程度）
    size_t frameIdx = 0;
    while (frameIdx + 1 < frames.size() && frames[frameIdx + 1].time <= currentTime)
        ++frameIdx;

    // --- ボーンごとにスキニング行列を計算 ---
    // (FBX出力では「姿勢行列」と「バインドポーズ」が両方必要)
    boneMatrices.clear();
    for (size_t i = 0; i < frames[frameIdx].pose.size(); ++i) {
        DirectX::XMMATRIX pose = frames[frameIdx].pose[i];    // 今フレームのボーン姿勢
        DirectX::XMMATRIX invBind = XMMatrixInverse(nullptr, bindPoses[i]); // バインドポーズの逆行列
        // スキニング用変換 = バインドポーズ逆行列 × 現在姿勢
        boneMatrices.push_back(invBind * pose);
        // ※なぜこうする？
        // 「バインドポーズ（初期Tポーズ）」を「現在アニメ姿勢」へ“相対的”に変換することで
        // 全ボーンの頂点を「正しい座標」へ動かすため
    }
    // --- バグ防止: ボーン数不一致を検出 ---
    if (bindPoses.size() != frames[frameIdx].pose.size()) {
        char msg[256];
        sprintf_s(msg, "[Error] ボーン数不一致！bind=%zu, pose=%zu\n",
            bindPoses.size(), frames[frameIdx].pose.size());
        OutputDebugStringA(msg);
    }
}

//--------------------------------------
// スキニング用の「各ボーン変換行列配列」を返す
// モデル描画時に参照される
//--------------------------------------
std::vector<XMMATRIX> Animator::GetSkinnedPose(const std::vector<XMMATRIX>& bindPoses) const {
    return boneMatrices; // もう計算済みなのでそのまま返す
}

//--------------------------------------
// 現在の各ボーン行列を返す（用途:デバッグや確認用）
//--------------------------------------
const std::vector<DirectX::XMMATRIX>& Animator::GetCurrentPose() const {
    return boneMatrices;
}

//--------------------------------------
// アニメーションデータを1個追加（動的に追加できる）
//--------------------------------------
void Animator::AddAnimation(const std::string& name, const std::vector<Keyframe>& keyframes) {
    if (keyframes.empty()) return;

    animations[name] = keyframes;

    // 最初のアニメなら currentAnim に設定
    if (currentAnim.empty()) {
        currentAnim = name;
        currentTime = 0.0;
    }

    // デバッグ出力
    char msg[128];
    sprintf_s(msg, "[Animator] アニメ '%s' を登録 (%zuフレーム)\n", name.c_str(), keyframes.size());
    OutputDebugStringA(msg);
}
