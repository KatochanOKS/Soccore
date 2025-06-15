#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include "BufferManager.h"           // ★ 追加
#include "FbxModelLoader.h"          // ★ 追加
class MeshRenderer : public Component {
public:
    int meshType = 0; // 0=Cube, 1=FBX
    int texIndex = -1;
    DirectX::XMFLOAT4 color = { 1,1,1,1 };

    BufferManager* modelBuffer = nullptr;                  // モデルごとのバッファ（FBX用）
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;      // モデルごとの頂点インデックス情報（FBX用）


    // ↓ C++11以降はデストラクタで解放推奨
    virtual ~MeshRenderer() {
        delete modelBuffer;
        delete vertexInfo;
    }
};
