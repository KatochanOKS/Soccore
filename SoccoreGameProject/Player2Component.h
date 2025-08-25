#pragma once
#include "Component.h"

class Player2Component : public Component {
public:
    void Update() override;
    float moveSpeed = 0.01f;   // プレイヤー2の移動速度
    float hp = 1.0f;         // HP（0.0～1.0）
    bool isGuarding = false; // ガード状態
};
