#pragma once
#include "Component.h"

class Player1Component : public Component {
public:
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;         // HP�i0.0�`1.0�j
    bool isGuarding = false; // �K�[�h���
};
