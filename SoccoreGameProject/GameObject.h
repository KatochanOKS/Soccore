#pragma once
#include "Transform.h"
#include <string>
#include"Colors.h"
class GameObject {
public:
    Transform transform;
    std::string name;
    int meshType = 0;    // 0=Cube, 1=Ground, ... �����g���p
    int meshIndex;
    int texIndex;    // SRV�q�[�v�ԍ��i-1�Ȃ疳�e�N�X�`���j
    XMFLOAT4 color;
    // �F�w�肠��
    GameObject(const std::string& name, int meshIdx = 0, int texIdx = -1, XMFLOAT4 color = Colors::White)
        : name(name), meshIndex(meshIdx), texIndex(texIdx), color(color) {
    }
    // �K�v�ɉ����ăf�X�g���N�^��R���|�[�l���g��
};
