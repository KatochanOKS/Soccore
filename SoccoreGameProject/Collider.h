#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Collider : public Component {
public:
    // ���[�J����Ԃ̒��S�E���a
    DirectX::XMFLOAT3 center = { 0,0,0 };
    DirectX::XMFLOAT3 size = { 1,1,1 }; // ���E�����E���s��

    // ���E���W�ł̍ŏ��E�ő�_��Ԃ�
    void GetAABBWorld(DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax);
};
