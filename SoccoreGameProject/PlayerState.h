// PlayerState.h
#pragma once

// どのプレイヤーでも使える汎用的な状態
enum class PlayerState {
    Idle,
    Move,
    Attack,
    Guard,
    Reaction,   // ダメージ受け中
    Dying,      // やられ
    Win         // 勝利
    // 必要なら他にも追加可能
};
