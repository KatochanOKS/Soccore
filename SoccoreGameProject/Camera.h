#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Camera {
public:
    void SetPosition(const DirectX::XMFLOAT3& pos) { m_position = pos; }
    void SetTarget(const DirectX::XMFLOAT3& tgt) { m_target = tgt; }

    DirectX::XMFLOAT3 GetPosition() const { return m_position; }

    DirectX::XMMATRIX GetViewMatrix() const {
        DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&m_position);
        DirectX::XMVECTOR target = DirectX::XMLoadFloat3(&m_target);
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
        return DirectX::XMMatrixLookAtLH(eye, target, up);
    }

    DirectX::XMMATRIX GetProjectionMatrix(float screenWidth, float screenHeight) const {
        return DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(90.0f),
            screenWidth / screenHeight,
            0.1f, 1000.0f
        );
    }
    float cameraYaw = 0.0f; // ÉJÉÅÉâÇÃâÒì]äpÅiYé≤é¸ÇËÅj
private:
    DirectX::XMFLOAT3 m_position = { 0, 10, -10 };
    DirectX::XMFLOAT3 m_target = { 0, 0, 0 };
    

};