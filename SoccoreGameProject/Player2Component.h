#pragma once
#include "Component.h"
#include "PlayerState.h" // ’Ç‰Á
#include <string>
#include <windows.h>

class Player2Component : public Component {
public:

    FILETIME lastWriteTime = {};  // ©’Ç‰Ái\‘¢‘Ì‰Šú‰»j

    PlayerState state = PlayerState::Idle;
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;
    float delayedHp = 1.0f;
    float maxHp = 1.0f;         // š’Ç‰Á
    bool isGuarding = false;
    float reactionTimer = 0.0f;
    std::string name = "Player2"; // š’Ç‰Á

    void LoadConfigFromLua();     // š’Ç‰Á
    void TakeDamage(float amount);
    void Start() override;        // šStart‚Å1‰ñ‚¾‚¯Lua“Ç
};

