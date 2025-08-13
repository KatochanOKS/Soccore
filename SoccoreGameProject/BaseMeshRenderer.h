// BaseMeshRenderer.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Renderer;

//----------------------------------------
// ���ׂẴ��b�V�������_���[�̊��N���X
// - Cube�AFBX�A�A�j��FBX�Ȃǂ�g�ݍ��킹����悤�ɂ���
// - Renderer::DrawObject �ȂǂŁA�[�I�� Draw()���Ăяo��
//----------------------------------------
class BaseMeshRenderer : public Component {
public:
    int texIndex = -1;                              // �e�N�X�`����SRV�G���ԍ�
    DirectX::XMFLOAT4 color = { 1,1,1,1 };          // �\���F (RGBA)

    // --- �`��֐� ---
    virtual void Draw(
        Renderer* renderer,               // �`��Ǘ��N���X
        size_t objIdx,                   // �I�u�W�F�N�g�ԍ�
        const DirectX::XMMATRIX& view,   // �r���[�s��
        const DirectX::XMMATRIX& proj    // �v���W�F�N�V�����s��
    ) = 0; // ���ۊ��̂��� pure virtual

    virtual ~BaseMeshRenderer() {}
};
