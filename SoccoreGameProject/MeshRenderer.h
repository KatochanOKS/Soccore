#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include "BufferManager.h"        // Vertex/SkinningVertex/BufferManager
#include "FbxModelLoader.h"       // VertexInfo/SkinningVertexInfo
#include <vector>
#include <string>
class Animator; // ← 前方宣言を必ず追加！

class MeshRenderer : public Component {
public:
    int meshType = 0;         // 0=Cube, 1=FBX, 2=スキンFBX など使い分け
    int texIndex = -1;
    DirectX::XMFLOAT4 color = { 1,1,1,1 };

    // --- バッファ類 ---
    BufferManager* modelBuffer = nullptr;    // 通常モデル用（Cubeや静的FBX）

    // --- 頂点・インデックス情報 ---
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;             // 静的モデル用

    BufferManager* skinBuffer = nullptr; // スキンメッシュ用
    BufferManager* skinningConstantBuffer = nullptr; // ★ボーンCBVバッファ（スキン用のみ）
    FbxModelLoader::SkinningVertexInfo* skinVertexInfo = nullptr; // スキン用

    Animator* animator = nullptr; // アニメ付きモデル用ポインタ

    // デストラクタで安全に解放（nullptrチェック付き！）
    virtual ~MeshRenderer() {
        if (modelBuffer) delete modelBuffer;
        if (skinBuffer) delete skinBuffer;
        if (vertexInfo) delete vertexInfo;
        if (skinVertexInfo) delete skinVertexInfo;
    }
};
