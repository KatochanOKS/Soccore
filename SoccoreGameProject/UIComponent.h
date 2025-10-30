// UIComponent.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

/// <summary>
/// UI�v�f�̈ʒu�E�T�C�Y�E�\����ԂȂǂ��Ǘ������{�R���|�[�l���g
/// </summary>

class UIComponent : public Component {
public:
    DirectX::XMFLOAT2 m_Position{ 0, 0 };   ///< ��ʍ��W (�s�N�Z��)
    DirectX::XMFLOAT2 m_Size{ 100, 100 };   ///< UI�T�C�Y
    float m_Layer = 0.0f;                   ///< �d�Ȃ菇�i���C���[�l�j
    bool m_Visible = true;                  ///< �\�����

    /// <summary>
    /// UI�`�揈���iRenderer�o�R�ŌĂяo���j
    /// </summary>
    
    virtual void Draw(class Renderer* renderer, size_t idx) {}

    /// <summary>
    /// UI��Ԃ̖��t���[���X�V
    /// </summary>
    
    virtual void Update(float dt) {}
};