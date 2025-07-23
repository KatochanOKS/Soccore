#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include "BufferManager.h"        // Vertex/SkinningVertex/BufferManager
#include "FbxModelLoader.h"       // VertexInfo/SkinningVertexInfo
#include <vector>
#include <string>

class MeshRenderer : public Component {
public:
    int meshType = 0;         // 0=Cube, 1=FBX, 2=�X�L��FBX �Ȃǎg������
    int texIndex = -1;
    DirectX::XMFLOAT4 color = { 1,1,1,1 };

    // --- �o�b�t�@�� ---
    BufferManager* modelBuffer = nullptr;    // �ʏ탂�f���p�iCube��ÓIFBX�j
    BufferManager* skinBuffer = nullptr;     // �X�L���A�j�����f���p

    // --- ���_�E�C���f�b�N�X��� ---
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;             // �ÓI���f���p
    FbxModelLoader::SkinningVertexInfo* skinInfo = nullptr;       // �X�L���A�j���p

    // --- �t���O ---
    bool hasSkinning = false;     // �X�L���A�j�����f���Ȃ�true



    // --- �{�[���֘A�iskinInfo�Əd������ꍇ�͏ȗ�OK�j ---
    std::vector<std::string> boneNames;
    std::vector<DirectX::XMMATRIX> bindPoses;

    std::vector<DirectX::XMMATRIX> boneMatrices;   // �{�[���̍ŏI�s��z��
    BufferManager* boneBuffer = nullptr;           // �{�[��CBV�p
    size_t boneCount = 0;                          // �{�[����


    // �f�X�g���N�^�ň��S�ɉ���inullptr�`�F�b�N�t���I�j
    virtual ~MeshRenderer() {
        if (modelBuffer) delete modelBuffer;
        if (skinBuffer)  delete skinBuffer;
        if (vertexInfo)  delete vertexInfo;
        if (skinInfo)    delete skinInfo;
        if (boneBuffer)  delete boneBuffer; // �ǉ�
    }
};
