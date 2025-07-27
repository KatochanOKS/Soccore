// Transform.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>
using namespace DirectX;

class Transform : public Component {
public:
    XMFLOAT3 position = { 0,0,0 };
    XMFLOAT3 rotation = { 0,0,0 };
    XMFLOAT3 scale = { 1,1,1 };

    // === ’Ç‰Á ===
    XMFLOAT3 GetPosition() const {
        return position;
    }

    void SetPosition(const XMFLOAT3& pos) {
        position = pos;
    }
    // ============

    XMMATRIX GetWorldMatrix() const {
        XMMATRIX matS = XMMatrixScaling(scale.x, scale.y, scale.z);
        XMMATRIX matR = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        XMMATRIX matT = XMMatrixTranslation(position.x, position.y, position.z);
        return matS * matR * matT;
    }
};

