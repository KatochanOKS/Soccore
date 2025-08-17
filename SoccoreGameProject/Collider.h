// Collider.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Transform;

class Collider : public Component {
public:
    DirectX::XMFLOAT3 center = { 0,0,0 };
    DirectX::XMFLOAT3 size = { 1,1,1 };

    // 親のTransformと同じ大きさ・中心に自動セット
    void AutoFitToTransform(Transform* tr);
    void GetAABBWorld(DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax);
};
