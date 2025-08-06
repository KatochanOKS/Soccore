// UIImage.h
#pragma once
#include "UIComponent.h"

class UIImage : public UIComponent {
public:
    int texIndex = -1;
    DirectX::XMFLOAT4 color{ 1, 1, 1, 1 }; // RGBA

    // 描画リクエスト（実際の描画処理はRendererでやる想定）
    void Draw(class Renderer* renderer, size_t idx) override;
};
