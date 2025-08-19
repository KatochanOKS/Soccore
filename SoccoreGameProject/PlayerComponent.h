#pragma once
#include "Component.h"

class PlayerComponent : public Component {
public:
    void Update() override;  // ここに毎フレームのロジックを書く

    // プレイヤーの追加情報があればここに
    float moveSpeed = 0.1f;
};
