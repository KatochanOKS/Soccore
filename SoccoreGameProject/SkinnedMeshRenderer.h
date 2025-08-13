// SkinnedMeshRenderer.h
#pragma once
#include "BaseMeshRenderer.h"
#include "BufferManager.h"
#include "FbxModelLoader.h"
#include "Animator.h"

class SkinnedMeshRenderer : public BaseMeshRenderer {
public:
    BufferManager* modelBuffer = nullptr; // �X�L�����b�V���p�o�b�t�@
    FbxModelLoader::SkinningVertexInfo* skinVertexInfo = nullptr; // �X�L�j���O���_���
    Animator* animator = nullptr;         // �A�j���[�V�����Ǘ��N���X

    ~SkinnedMeshRenderer() override;   // �� ; �ŏI��邾���I�i�{�͖̂����j
    void Draw() override;              // �� ; �ŏI��邾���I�i�{�͖̂����j
};
