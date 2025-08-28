#pragma once
#include "Component.h"
#include "PlayerState.h" // 追加

class Player2Component : public Component {
public:
    PlayerState state = PlayerState::Idle; // 状態変数
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;
    float delayedHp = 1.0f;
    bool isGuarding = false;
    float reactionTimer = 0.0f; // Reaction中のタイマー

    void TakeDamage(float amount); // 被ダメージ用
};
