#pragma once
#include "Component.h"
#include <DirectXMath.h>

class MeshRenderer : public Component {
public:
    int meshType = 0; // 0=Cube, 1=FBX
    int texIndex = -1;
    DirectX::XMFLOAT4 color = { 1,1,1,1 };
};
