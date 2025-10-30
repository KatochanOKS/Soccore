#pragma once
#include "Component.h"
#include <string>
#include <vector>

/// <summary>
/// �X���b�g���[���̉�]�E��~�E�o�ڔ�����Ǘ�����R���|�[�l���g
/// </summary>
class ReelComponent : public Component {
public:
    float m_Speed = -0.01f;         ///< ���[���̉�]�X�s�[�h
    float m_Angle = 0.0f;           ///< ���݂̊p�x�i���W�A���j
    bool m_IsSpinning = true;       ///< ��]�����ǂ���
    bool m_IsStopping = false;      ///< ��~�����ǂ���
    int m_StopIndex = -1;           ///< �~�܂����Ƃ��̃R�}�ԍ�
    static constexpr int s_NumSymbols = 8; ///< �R�}���i�萔�j
    std::vector<std::string> m_Symbols = { "7", "BAR", "�`�F���[", "�X�C�J", "���v���C", "�x��", "�u�h�E", "�n�Y��" };

    /// <summary>
    /// ���t���[���̃��[����ԍX�V
    /// </summary>
    
    void Update() override;

    /// <summary>
    /// ���݂̊p�x����o�ڂ𔻒肵�A��~�������s��
    /// </summary>

    void JudgeSymbol();

    /// <summary>
    /// ��]�p��␳���ă��[�����s�^�b�Ǝ~�߂�
    /// </summary>
   
    void StopAndSnap();
};