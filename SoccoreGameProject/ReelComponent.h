#pragma once
#include "Component.h"
#include <string>
#include <vector>

/// <summary>
/// スロットリールの回転・停止・出目判定を管理するコンポーネント
/// </summary>
class ReelComponent : public Component {
public:
    float m_Speed = -0.01f;         ///< リールの回転スピード
    float m_Angle = 0.0f;           ///< 現在の角度（ラジアン）
    bool m_IsSpinning = true;       ///< 回転中かどうか
    bool m_IsStopping = false;      ///< 停止中かどうか
    int m_StopIndex = -1;           ///< 止まったときのコマ番号
    static constexpr int s_NumSymbols = 8; ///< コマ数（定数）
    std::vector<std::string> m_Symbols = { "7", "BAR", "チェリー", "スイカ", "リプレイ", "ベル", "ブドウ", "ハズレ" };

    /// <summary>
    /// 毎フレームのリール状態更新
    /// </summary>
    
    void Update() override;

    /// <summary>
    /// 現在の角度から出目を判定し、停止処理を行う
    /// </summary>

    void JudgeSymbol();

    /// <summary>
    /// 回転角を補正してリールをピタッと止める
    /// </summary>
   
    void StopAndSnap();
};