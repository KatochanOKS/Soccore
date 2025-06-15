#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include "BufferManager.h"           // �� �ǉ�
#include "FbxModelLoader.h"          // �� �ǉ�
class MeshRenderer : public Component {
public:
    int meshType = 0; // 0=Cube, 1=FBX
    int texIndex = -1;
    DirectX::XMFLOAT4 color = { 1,1,1,1 };

    BufferManager* modelBuffer = nullptr;                  // ���f�����Ƃ̃o�b�t�@�iFBX�p�j
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;      // ���f�����Ƃ̒��_�C���f�b�N�X���iFBX�p�j


    // �� C++11�ȍ~�̓f�X�g���N�^�ŉ������
    virtual ~MeshRenderer() {
        delete modelBuffer;
        delete vertexInfo;
    }
};
