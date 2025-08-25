#pragma once
#include "Component.h"

class Player1Component : public Component {
public:
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;         // HP（0.0～1.0）
    bool isGuarding = false; // ガード状態
};
