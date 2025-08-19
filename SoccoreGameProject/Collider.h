#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Transform;

class Collider : public Component {
public:
    DirectX::XMFLOAT3 center = { 0,0,0 }; // ���[�J�����S
    DirectX::XMFLOAT3 size = { 1,1,1 };   // ���[�J���傫��

    // tr�͂���Collider�̐eTransform
    void GetAABBWorld(const Transform* tr, DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax) const;
};
