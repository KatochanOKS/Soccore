#pragma once
#include <DirectXMath.h>
using namespace DirectX;

// 3D位置・回転・スケールを持つクラス
class Transform {
public:
    XMFLOAT3 position = { 0, 0, 0 };
    XMFLOAT3 rotation = { 0, 0, 0 }; // ラジアン
    XMFLOAT3 scale = { 1, 1, 1 };

    // ワールド行列を計算
    XMMATRIX GetWorldMatrix() const {
        XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
        XMMATRIX R = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
        return S * R * T;
    }
};
