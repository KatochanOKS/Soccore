#pragma once
#include "BaseMeshRenderer.h"
#include "BufferManager.h"
#include "FbxModelLoader.h"

// ------------------------------
// �ÓI�i�A�j���Ȃ��j���b�V����`�悷���p�N���X
// Cube, ���ʂ�FBX, �n�ʂȂ�
// ------------------------------
class StaticMeshRenderer : public BaseMeshRenderer {
public:
    BufferManager* modelBuffer = nullptr;               // ���_�E�C���f�b�N�X�o�b�t�@
    FbxModelLoader::VertexInfo* vertexInfo = nullptr;   // ���f���f�[�^�i���_�E�C���f�b�N�X�z��j

    bool isQuad2D = false;
    bool isSkySphere = false; // 2D���ʗp�E���̗p�̃t���O�iQuad��Sphere�Ȃ���ʂȏ���������j
	bool isSphere = false; // ���̃��b�V�����ǂ����i�X�J�C�h�[���p�j
    // �f�X�g���N�^�inew�������͕̂K��delete�I�j
    ~StaticMeshRenderer() override;

    // �`�揈���i��Ŏ����������I�j
    void Draw() override;
};
