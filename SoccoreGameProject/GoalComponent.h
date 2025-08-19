#pragma once
#include "Component.h"
#include <DirectXMath.h>

class GoalComponent : public Component {
public:

    void Update() override;
    // ���[���h��ԂŔ���
    bool CheckGoal(const DirectX::XMFLOAT3& ballPos);
};
