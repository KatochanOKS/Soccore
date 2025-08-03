#pragma once
#include "Component.h"
#include <DirectXMath.h>

// ------------------------------
// �S�Ẵ��b�V�������_���[�́u���ʕ����v���������e�N���X
// �E�F��e�N�X�`���ԍ��Ȃǋ��ʃf�[�^
// �EDraw�֐��͕K���q�N���X�Œ�`�i=0�j
// ------------------------------
class BaseMeshRenderer : public Component {
public:
    int texIndex = -1;  // �e�N�X�`���C���f�b�N�X�i�g��Ȃ��ꍇ��-1�j
    DirectX::XMFLOAT4 color = { 1, 1, 1, 1 }; // �F�f�[�^�iR,G,B,A�j

    // �u�`��֐��v�����z�֐��ɂ��Ă����i=0�́u�������z�֐��v�j
    // �� �K���q�N���X�Œ��g�������Ȃ��Ƃ����Ȃ��I
    virtual void Draw() = 0;

    // �f�X�g���N�^�i�K��virtual! �p�����̔j���R��o�O�h�~�j
    virtual ~BaseMeshRenderer() {}
};
