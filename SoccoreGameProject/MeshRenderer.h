#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include "BufferManager.h"        // Vertex/SkinningVertex/BufferManager
#include "FbxModelLoader.h"       // VertexInfo/SkinningVertexInfo
#include <vector>
#include <string>

class MeshRenderer : public Component {
public:
    int meshType = 0;         // 0=Cube, 1=FBX, 2=スキンFBX など使い分け
    int texIndex = -1;
    DirectX::XMFLOAT4 color = { 1,1,1,1 };

    // --- バッファ類 ---
    BufferManager* modelBuffer = nullptr;    // 通常モデル用（Cubeや静的FBX）
    BufferManager* skinBuffer = nullptr;     // スキンアニメモデル用

    // --- 頂点・インデックス情報 ---
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;             // 静的モデル用
    FbxModelLoader::SkinningVertexInfo* skinInfo = nullptr;       // スキンアニメ用

    // --- フラグ ---
    bool hasSkinning = false;     // スキンアニメモデルならtrue



    // --- ボーン関連（skinInfoと重複する場合は省略OK） ---
    std::vector<std::string> boneNames;
    std::vector<DirectX::XMMATRIX> bindPoses;

    std::vector<DirectX::XMMATRIX> boneMatrices;   // ボーンの最終行列配列
    BufferManager* boneBuffer = nullptr;           // ボーンCBV用
    size_t boneCount = 0;                          // ボーン数


    // デストラクタで安全に解放（nullptrチェック付き！）
    virtual ~MeshRenderer() {
        if (modelBuffer) delete modelBuffer;
        if (skinBuffer)  delete skinBuffer;
        if (vertexInfo)  delete vertexInfo;
        if (skinInfo)    delete skinInfo;
        if (boneBuffer)  delete boneBuffer; // 追加
    }
};
