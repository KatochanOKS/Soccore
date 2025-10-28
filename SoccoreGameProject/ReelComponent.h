#pragma once
#include "Component.h"
#include <string>
#include <vector>

class ReelComponent : public Component {
public:
    float speed = -0.01f;         // 回転スピード
    float angle = 0.0f;         // 現在の角度（ラジアン）
    bool isSpinning = true;     // 回転中かどうか
	bool isStopping = false;    // 停止中かどうか
    int stopIndex = -1;         // 止まったときのコマ番号
    static constexpr int numSymbols = 8; // コマ数
    std::vector<std::string> symbols = { "7", "BAR", "チェリー", "スイカ", "リプレイ", "ベル", "ブドウ", "ハズレ" };

    void Update() override;
    void JudgeSymbol();     // 出目判定（ピタッと止める）
    void StopAndSnap();     // 回転角をピタッと補正して止める
};
