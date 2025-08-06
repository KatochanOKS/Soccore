// UIComponent.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

class UIComponent : public Component {
public:
    DirectX::XMFLOAT2 position{ 0, 0 }; // ��ʍ��W (�s�N�Z��)
    DirectX::XMFLOAT2 size{ 100, 100 }; // �T�C�Y
    float layer = 0.0f; // �d�Ȃ菇
    bool visible = true;

    virtual void Draw(class Renderer* renderer, size_t idx) {}
    virtual void Update(float dt) {}
};
