#pragma once
#include "Component.h"
#include <string>
#include <vector>

class ReelComponent : public Component {
public:
    float speed = -0.01f;         // ��]�X�s�[�h
    float angle = 0.0f;         // ���݂̊p�x�i���W�A���j
    bool isSpinning = true;     // ��]�����ǂ���
	bool isStopping = false;    // ��~�����ǂ���
    int stopIndex = -1;         // �~�܂����Ƃ��̃R�}�ԍ�
    static constexpr int numSymbols = 8; // �R�}��
    std::vector<std::string> symbols = { "7", "BAR", "�`�F���[", "�X�C�J", "���v���C", "�x��", "�u�h�E", "�n�Y��" };

    void Update() override;
    void JudgeSymbol();     // �o�ڔ���i�s�^�b�Ǝ~�߂�j
    void StopAndSnap();     // ��]�p���s�^�b�ƕ␳���Ď~�߂�
};
