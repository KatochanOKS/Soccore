#pragma once
#include "BaseMeshRenderer.h"
#include "BufferManager.h"
#include "FbxModelLoader.h"
#include "Animator.h"

/// <summary>
/// スキンメッシュ（ボーンアニメ対応メッシュ）を描画する専用クラス。
/// FBXのボーンアニメやキャラクターなどに利用。
/// </summary>
class SkinnedMeshRenderer : public BaseMeshRenderer {
public:
    BufferManager* m_ModelBuffer = nullptr; ///< スキンメッシュ用バッファ
    FbxModelLoader::SkinningVertexInfo* m_SkinVertexInfo = nullptr; ///< スキニング頂点情報
    Animator* m_Animator = nullptr;         ///< アニメーション管理クラス
    BufferManager* m_BoneCB = nullptr;      ///< ボーン用定数バッファ（各キャラごと）

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~SkinnedMeshRenderer() override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw() override;
};