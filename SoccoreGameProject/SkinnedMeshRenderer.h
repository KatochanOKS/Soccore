// SkinnedMeshRenderer.h
#pragma once
#include "BaseMeshRenderer.h"
#include "BufferManager.h"
#include "FbxModelLoader.h"
#include "Animator.h"

class SkinnedMeshRenderer : public BaseMeshRenderer {
public:
    BufferManager* modelBuffer = nullptr; // スキンメッシュ用バッファ
    FbxModelLoader::SkinningVertexInfo* skinVertexInfo = nullptr; // スキニング頂点情報
    Animator* animator = nullptr;         // アニメーション管理クラス

    ~SkinnedMeshRenderer() override;   // ← ; で終わるだけ！（本体は無し）
    void Draw() override;              // ← ; で終わるだけ！（本体は無し）
};
