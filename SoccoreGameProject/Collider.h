#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Collider : public Component {
public:
    // ローカル空間の中心・半径
    DirectX::XMFLOAT3 center = { 0,0,0 };
    DirectX::XMFLOAT3 size = { 1,1,1 }; // 幅・高さ・奥行き

    // 世界座標での最小・最大点を返す
    void GetAABBWorld(DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax);
};
