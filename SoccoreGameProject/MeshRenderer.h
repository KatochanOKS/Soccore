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

    // --- ���_�E�C���f�b�N�X��� ---
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;             // �ÓI���f���p

    BufferManager* skinBuffer = nullptr; // �X�L�����b�V���p
    FbxModelLoader::SkinningVertexInfo* skinVertexInfo = nullptr; // �X�L���p

    // �f�X�g���N�^�ň��S�ɉ���inullptr�`�F�b�N�t���I�j
    virtual ~MeshRenderer() {
        if (modelBuffer) delete modelBuffer;
        if (skinBuffer) delete skinBuffer;
        if (vertexInfo) delete vertexInfo;
        if (skinVertexInfo) delete skinVertexInfo;
    }
};
