#include "Animator.h"
#define NOMINMAX      // ←これが必須！！
#include <windows.h>
#include <algorithm>  // min, max, std::min, std::maxが両立できる
#include <DirectXMath.h>
#include <cassert>
#include <sstream>
using namespace DirectX;


//◆ アニメーション導入手順まとめ
//
//Animatorコンポーネントのアタッチ
//
//各キャラクターのGameObjectにAnimatorをAddComponentする
//
//FBXファイルからアニメーションデータ（キーフレーム）を読み込む
//
//FBXモデル＋各アニメごとに
//
//FbxModelLoader::LoadAnimationOnly(fbxファイル名, std::vector<Keyframe>&, double& アニメ長さ)
//
//読み込んだキーフレーム配列をAnimator::AddAnimation("アニメ名", キーフレーム配列)で登録
//
//ボーン名・バインドポーズ行列配列も同時に取得してセット
//
//モデル読込時にAnimator::SetAnimationsで、アニメデータ・ボーン名・バインドポーズをセット
//
//再生開始アニメを選ぶ
//
//Animator::SetAnimation("Idle", true) などで初期アニメをセット（通常はIdle）
//
//毎フレーム、Animator::Updateを呼ぶ
//
//GameSceneのUpdate内やコンポーネントUpdateで
//animator->Update(フレーム秒数);
//を必ず呼び出し、ボーン姿勢を更新
//
//状態遷移時（攻撃・ガードなど）でSetAnimationを呼ぶ
//
//例：攻撃ボタン入力でSetAnimation("Kick", false);
//
//防御時でSetAnimation("BodyBlock", true);
//
//状態管理（ステートマシン）と組み合わせると「どのタイミングでどのアニメに切り替えるか」を柔軟に制御できる
//
//描画時はAnimatorのボーン行列（GetSkinnedPose）をスキニング処理に渡す
//
//スキンメッシュ描画パイプライン内で
//animator->GetSkinnedPose(...)
//から取得した行列をシェーダーに渡す
//
//【補足】
//
//新しいアニメを追加したい場合は、
//AddAnimation("アニメ名", キーフレーム配列); を呼ぶだけでOK
//
//モデル・ボーン・バインドポーズの整合性が取れていることが必須
//
//アニメーション状態管理は「プレイヤーステートマシン」で一元制御するとベスト


//--------------------------------------
// コンストラクタ
//--------------------------------------
Animator::Animator() {
    // 必要ならここで初期値セット
    // 全メンバは初期化子で初期化済み
}

//--------------------------------------
// アニメ・ボーン・バインドポーズ一括セット
// moveセマンティクスで不要なコピーを抑止！
//--------------------------------------
void Animator::SetAnimations(
    std::unordered_map<std::string, std::vector<Keyframe>>&& anims,
    std::vector<std::string>&& bones,
    std::vector<XMMATRIX>&& bindPoseMatrices
) {
    animations = std::move(anims);       // ムーブで効率的に受け渡し
    boneNames = std::move(bones);
    bindPoses = std::move(bindPoseMatrices);

    // 最初のアニメをデフォルト再生
    if (!animations.empty())
        currentAnim = animations.begin()->first;
    currentTime = 0.0;
    isPlaying = true;

    // ボーン数分の行列配列を初期化
    boneMatrices.clear();
    if (!boneNames.empty())
        boneMatrices.resize(boneNames.size(), XMMatrixIdentity());
}

//--------------------------------------
// アニメーションを切り替える
// 同一アニメへの連続再生は抑止
//--------------------------------------
void Animator::SetAnimation(const std::string& animName, bool loop_) {
    // 同じアニメで再生中なら何もしない（割り込み再生回避）
    if (currentAnim == animName && isPlaying)
        return;

    auto it = animations.find(animName);
    if (it != animations.end()) {
        currentAnim = animName;
        currentTime = 0.0;
        loop = loop_;
        isPlaying = true;
    }
    else {
        std::ostringstream oss;
        oss << "[Animator][Error] 未登録アニメ: " << animName << "\n";
        OutputDebugStringA(oss.str().c_str());
    }
}

//--------------------------------------
// 指定時刻に一番近いキーフレームインデックスを返す（境界安全）
//--------------------------------------
size_t Animator::FindKeyframeIndex(const std::vector<Keyframe>& frames, double time) const {
    size_t frameIdx = 0;
    while (frameIdx + 1 < frames.size() && frames[frameIdx + 1].time <= time)
        ++frameIdx;
    return frameIdx;
}

//--------------------------------------
// 毎フレーム「現在の時刻」に合わせて各ボーン行列を計算
// 例外/不正値も堅牢に検出
//--------------------------------------
void Animator::Update(float deltaTime) {
    if (!isPlaying || animations.count(currentAnim) == 0) return;
    const auto& frames = animations.at(currentAnim);

    if (frames.empty()) {
        OutputDebugStringA("[Animator][Error] 現在アニメのキーフレームが0です\n");
        isPlaying = false;
        return;
    }

    currentTime += deltaTime * 0.8;
    double animLength = frames.back().time;
    if (animLength <= 0.0) {
        OutputDebugStringA("[Animator][Error] アニメ長さ0\n");
        isPlaying = false;
        return;
    }

    // ループまたは停止
    if (currentTime > animLength) {
        if (loop) {
            currentTime = fmod(currentTime, animLength);
        }
        else {
            currentTime = animLength;
            isPlaying = false; // 1回きりアニメは停止
        }
    }

    // キーフレーム番号取得（必ず安全な範囲）
    size_t frameIdx = FindKeyframeIndex(frames, currentTime);

    // ボーン数一致チェック
    if (bindPoses.size() != frames[frameIdx].pose.size()) {
        char msg[256];
        sprintf_s(msg, "[Animator][Error] ボーン数不一致！bind=%zu, pose=%zu\n",
            bindPoses.size(), frames[frameIdx].pose.size());
        OutputDebugStringA(msg);
        // フェイルセーフ：小さい方に合わせる
    }
    size_t safeBoneCount = std::min(bindPoses.size(), frames[frameIdx].pose.size());

    // --- ボーンごとにスキニング行列を計算 ---
    boneMatrices.clear();
    for (size_t i = 0; i < safeBoneCount; ++i) {
        XMMATRIX pose = frames[frameIdx].pose[i];      // 今フレームのボーン姿勢
        XMMATRIX invBind = XMMatrixInverse(nullptr, bindPoses[i]); // バインドポーズ逆行列
        // スキニング用変換 = バインドポーズ逆行列 × 現在姿勢
        boneMatrices.push_back(invBind * pose);
        // ※バインドポーズ（初期Tポーズ）→現在アニメ姿勢への相対変換で
        // FBXモデルの正しいスキニングを実現！
    }
}

//--------------------------------------
// スキニング用の「各ボーン変換行列配列」を返す
// （すでにUpdateで計算済み）
//--------------------------------------
std::vector<XMMATRIX> Animator::GetSkinnedPose(const std::vector<XMMATRIX>& /*bindPoses*/) const {
    return boneMatrices;
}

//--------------------------------------
// 現在の各ボーン行列を返す（用途:デバッグや確認用）
//--------------------------------------
const std::vector<XMMATRIX>& Animator::GetCurrentPose() const {
    return boneMatrices;
}

//--------------------------------------
// アニメーションデータを1個追加（動的追加もOK）
//--------------------------------------
void Animator::AddAnimation(const std::string& name, const std::vector<Keyframe>& keyframes) {
    if (keyframes.empty()) {
        OutputDebugStringA("[Animator][Warn] 0フレームアニメは登録しません\n");
        return;
    }

    animations[name] = keyframes;

    // 最初のアニメなら currentAnim に設定
    if (currentAnim.empty()) {
        currentAnim = name;
        currentTime = 0.0;
    }

    char msg[128];
    sprintf_s(msg, "[Animator] アニメ '%s' を登録 (%zuフレーム)\n", name.c_str(), keyframes.size());
    OutputDebugStringA(msg);
}
