// Collider.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Transform;

class Collider : public Component {
public:
    DirectX::XMFLOAT3 center = { 0,0,0 };
    DirectX::XMFLOAT3 size = { 1,1,1 };

    // �e��Transform�Ɠ����傫���E���S�Ɏ����Z�b�g
    void AutoFitToTransform(Transform* tr);
    void GetAABBWorld(DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax);
};
