#pragma once
#include "Component.h"
#include <DirectXMath.h>

class GoalComponent : public Component {
public:

    void Update() override;
    // ƒ[ƒ‹ƒh‹óŠÔ‚Å”»’è
    bool CheckGoal(const DirectX::XMFLOAT3& ballPos);
};
