// UIComponent.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

class UIComponent : public Component {
public:
    DirectX::XMFLOAT2 position{ 0, 0 }; // 画面座標 (ピクセル)
    DirectX::XMFLOAT2 size{ 100, 100 }; // サイズ
    float layer = 0.0f; // 重なり順
    bool visible = true;

    virtual void Draw(class Renderer* renderer, size_t idx) {}
    virtual void Update(float dt) {}
};
