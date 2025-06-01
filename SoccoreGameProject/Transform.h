#pragma once
#include <DirectXMath.h>
using namespace DirectX;

// 3D�ʒu�E��]�E�X�P�[�������N���X
class Transform {
public:
    XMFLOAT3 position = { 0, 0, 0 };
    XMFLOAT3 rotation = { 0, 0, 0 }; // ���W�A��
    XMFLOAT3 scale = { 1, 1, 1 };

    // ���[���h�s����v�Z
    XMMATRIX GetWorldMatrix() const {
        XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
        XMMATRIX R = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
        return S * R * T;
    }
};
