#pragma once
#include "Component.h"
#include <DirectXMath.h>

class SoccerBallComponent : public Component {
public:
    DirectX::XMFLOAT3 velocity = { 0, 0, 0 };
    void Update() override;
    void Kick(float angleRad, float power);
};
