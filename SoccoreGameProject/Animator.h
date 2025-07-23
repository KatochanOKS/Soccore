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

    std::string currentAnim;  // 現在のアニメ名
    double currentTime = 0.0; // アニメ再生位置
    bool isPlaying = true;    // 再生中フラグ

    Animator();

    // アニメ・ボーン情報をセット（初期化時に呼ぶ）
    void SetAnimations(const std::unordered_map<std::string, std::vector<Keyframe>>& anims, const std::vector<std::string>& bones);

    // 再生アニメーションを切り替える（Walk, Jump等）
    void SetAnimation(const std::string& animName);

    // 毎フレーム、現在のボーン行列を更新する（※基底のUpdateとは独立）
    void Update(float deltaTime);

    // 現在のボーン行列（描画用）
    const std::vector<DirectX::XMMATRIX>& GetCurrentPose() const;
};
