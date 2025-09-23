#pragma once
#include "Component.h"
#include "PlayerState.h" // �ǉ�
#include <string>
#include <windows.h>

class Player2Component : public Component {
public:

    FILETIME lastWriteTime = {};  // ���ǉ��i�\���̏������j

    PlayerState state = PlayerState::Idle;
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;
    float delayedHp = 1.0f;
    float maxHp = 1.0f;         // ���ǉ�
    bool isGuarding = false;
    float reactionTimer = 0.0f;
    std::string name = "Player2"; // ���ǉ�

    void LoadConfigFromLua();     // ���ǉ�
    void TakeDamage(float amount);
    void Start() override;        // ��Start��1�񂾂�Lua�Ǎ�
};

