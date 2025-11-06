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
    BufferManager* m_ModelBuffer = nullptr;               ///< 頂点・インデックスバッファ
    FbxModelLoader::VertexInfo* m_VertexInfo = nullptr;   ///< モデルデータ（頂点・インデックス配列）
    bool IsSkySphere = false; ///< モデルデータ（頂点・インデックス配列）
	
    /// <summary>
    /// デストラクタ（newしたものは必ずdelete）
    /// </summary>
    
    ~StaticMeshRenderer() override;

	/// <summary>
    /// 描画処理
    /// </summary>
    
	void Draw() override;
};
