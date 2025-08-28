#pragma once
#include "Component.h"
#include "PlayerState.h" // �ǉ�

class Player2Component : public Component {
public:
    PlayerState state = PlayerState::Idle; // ��ԕϐ�
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;
    float delayedHp = 1.0f;
    bool isGuarding = false;
    float reactionTimer = 0.0f; // Reaction���̃^�C�}�[

    void TakeDamage(float amount); // ��_���[�W�p
};
