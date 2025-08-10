#pragma once
#include "BaseMeshRenderer.h"
#include "BufferManager.h"
#include "FbxModelLoader.h"

// ------------------------------
// 静的（アニメなし）メッシュを描画する専用クラス
// Cube, 普通のFBX, 地面など
// ------------------------------
class StaticMeshRenderer : public BaseMeshRenderer {
public:
    BufferManager* modelBuffer = nullptr;               // 頂点・インデックスバッファ
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;   // モデルデータ（頂点・インデックス配列）

    bool isQuad2D = false;
    bool isSkySphere = false; // 2D平面用・球体用のフラグ（QuadやSphereなら特別な処理をする）

    // デストラクタ（newしたものは必ずdelete！）
    ~StaticMeshRenderer() override;

    // 描画処理（後で実装例を説明！）
    void Draw() override;
};
