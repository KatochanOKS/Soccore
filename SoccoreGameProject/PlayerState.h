// PlayerState.h
#pragma once

// �ǂ̃v���C���[�ł��g����ėp�I�ȏ��
enum class PlayerState {
    Idle,
    Move,
    Attack,
    Guard,
    Reaction,   // �_���[�W�󂯒�
    Dying,      // ����
    Win         // ����
    // �K�v�Ȃ瑼�ɂ��ǉ��\
};
