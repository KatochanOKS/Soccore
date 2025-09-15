#pragma once
#include "Component.h"
#include "PlayerState.h" // ←追加
#include "Scene.h"
#include <memory>
#include <string>



class Player1Component : public Component {
public:
    PlayerState state = PlayerState::Idle;
    void Update() override;
    float moveSpeed = 0.01f;
    float hp = 1.0f;
    float delayedHp = 1.0f;
    float maxHp = 1.0f;           // ★ここを追加
    bool isGuarding = false;
    float reactionTimer = 0.0f;

    std::string name = "Player1";
    void LoadConfigFromLua();
    void TakeDamage(float amount);

    void Start() override;
};

