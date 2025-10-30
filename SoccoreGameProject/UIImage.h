// UIImage.h
#pragma once
#include "UIComponent.h"

/// <summary>
/// �摜�\����p��UI�R���|�[�l���g�i�e�N�X�`��ID�E�F�t���j
/// </summary>

class UIImage : public UIComponent {
public:
    int m_TexIndex = -1;                    ///< �g�p����e�N�X�`���̃C���f�b�N�X
    DirectX::XMFLOAT4 m_Color{ 1, 1, 1, 1 };///< RGBA�J���[

    /// <summary>
    /// �摜UI�̕`�惊�N�G�X�g�i���ۂ̕`�揈����Renderer�ōs���j
    /// </summary>
    
    void Draw(class Renderer* renderer, size_t idx) override;
};