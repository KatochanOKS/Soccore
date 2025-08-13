// UIImage.h
#pragma once
#include "UIComponent.h"

class UIImage : public UIComponent {
public:
    int texIndex = -1;
    DirectX::XMFLOAT4 color{ 1, 1, 1, 1 }; // RGBA

    // �`�惊�N�G�X�g�i���ۂ̕`�揈����Renderer�ł��z��j
    void Draw(class Renderer* renderer, size_t idx) override;
};
