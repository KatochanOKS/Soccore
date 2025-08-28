#pragma once
#include "Component.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <DirectXMath.h>

//--------------------------------------
// FBXアニメーション再生用コンポーネント
//--------------------------------------
class Animator : public Component {
public:
    // キーフレーム情報（各時刻ごとに全ボーンの行列セット）
    struct Keyframe {
        double time; // 再生時刻（秒単位）
        std::vector<DirectX::XMMATRIX> pose; // 全ボーンの姿勢行列
    };

    // アニメーションごとのキーフレーム配列
    std::unordered_map<std::string, std::vector<Keyframe>> animations; // アニメ名→キーフレーム配列

    std::vector<std::string> boneNames;                   // ボーン名リスト
    std::vector<DirectX::XMMATRIX> boneMatrices;           // 今の全ボーンの姿勢

    std::vector<DirectX::XMMATRIX> bindPoses; // ★バインドポーズを保存しておく

    std::string currentAnim;  // 現在のアニメ名
    double currentTime = 0.0; // アニメ再生位置

    bool loop = true;  // デフォルトはループ再生
    bool isPlaying = true;  // 今再生中か

    // --- コンストラクタ ---
    Animator();

    // --- アニメ/ボーン/バインドポーズのセット（moveセマンティクス対応） ---
    void SetAnimations(
        std::unordered_map<std::string, std::vector<Keyframe>>&& anims,
        std::vector<std::string>&& bones,
        std::vector<DirectX::XMMATRIX>&& bindPoseMatrices
    );

    // --- アニメーションの切り替え ---
    void SetAnimation(const std::string& animName, bool loop = true);

    // --- フレームごとにアニメを進める ---
    void Update(float deltaTime);

    // --- 現在のボーン行列（描画用） ---
    const std::vector<DirectX::XMMATRIX>& GetCurrentPose() const;

    // --- スキニング用（バインドポーズ補正つき） ---
    std::vector<DirectX::XMMATRIX> GetSkinnedPose(const std::vector<DirectX::XMMATRIX>& bindPoses) const;

    // --- アニメーション追加（単体でも追加可能） ---
    void AddAnimation(const std::string& name, const std::vector<Keyframe>& keyframes);

private:
    // 内部的な「再生中フレーム番号」計算
    size_t FindKeyframeIndex(const std::vector<Keyframe>& frames, double time) const;
};
