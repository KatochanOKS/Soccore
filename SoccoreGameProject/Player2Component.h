#pragma once
#include "Component.h"

class Player2Component : public Component {
public:
    void Update() override;
    float moveSpeed = 0.01f;   // �v���C���[2�̈ړ����x
    float hp = 1.0f;         // HP�i0.0�`1.0�j
    bool isGuarding = false; // �K�[�h���
};
