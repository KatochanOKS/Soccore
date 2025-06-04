#pragma once
#include "Transform.h"
#include <string>
#include"Colors.h"
class GameObject {
public:
    Transform transform;
    std::string name;
    int meshType = 0;    // 0=Cube, 1=Ground, ... 将来拡張用
    int meshIndex;
    int texIndex;    // SRVヒープ番号（-1なら無テクスチャ）
    XMFLOAT4 color;
    // 色指定あり
    GameObject(const std::string& name, int meshIdx = 0, int texIdx = -1, XMFLOAT4 color = Colors::White)
        : name(name), meshIndex(meshIdx), texIndex(texIdx), color(color) {
    }
    // 必要に応じてデストラクタやコンポーネントも
};
