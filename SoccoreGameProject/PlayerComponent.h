#pragma once
#include "Component.h"

class PlayerComponent : public Component {
public:
    void Update() override;  // �����ɖ��t���[���̃��W�b�N������

    // �v���C���[�̒ǉ���񂪂���΂�����
    float moveSpeed = 0.1f;
};
