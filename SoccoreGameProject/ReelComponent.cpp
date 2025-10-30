#include "ReelComponent.h"
#include "Transform.h"
#include "GameObject.h"
#include <Windows.h>
#include <DirectXMath.h>
#include <cmath>
#include <cstdio>

/// <summary>
/// ���[���̉�]�E��~�����𖈃t���[���X�V����
/// </summary>
void ReelComponent::Update() {
    if (m_IsSpinning) {
        m_Angle += m_Speed;

        // 2�΃��[�v
        if (m_Angle > DirectX::XM_PI * 2.0f) m_Angle -= DirectX::XM_PI * 2.0f;
        if (m_Angle < 0) m_Angle += DirectX::XM_PI * 2.0f;

        // ��~�{�^������
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            m_IsStopping = true;
        }

        if (m_IsStopping) {
            // �R�}�̒��S���B����
            const int N = s_NumSymbols;
            float unitPrev = fmodf((m_Angle - m_Speed) / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f); // �O�t���[��
            float unitNow = fmodf(m_Angle / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f);

            // �u-0.5f/N�v�Œ������
            unitPrev = fmodf(unitPrev - 0.5f / N + 1.0f, 1.0f);
            unitNow = fmodf(unitNow - 0.5f / N + 1.0f, 1.0f);

            int idxPrev = (int)(unitPrev * N);
            int idxNow = (int)(unitNow * N);

            if (idxPrev != idxNow) {
                // �R�}�̒��S��ʉ߂����^�C�~���O�I
                StopAndSnap(); // �s�^�b�ƃR�}���S�փX�i�b�v���o�ڔ���
                m_IsStopping = false;
                m_IsSpinning = false;
            }
        }
    }
    // Transform���f
    if (auto* tr = gameObject->GetComponent<Transform>()) {
        tr->rotation.x = m_Angle;
    }
}

/// <summary>
/// �R�}�̒��S�Ɋp�x���X�i�b�v���A��~�C���f�b�N�X�Əo�ڂ��m�肷��
/// </summary>
void ReelComponent::StopAndSnap() {
    int N = s_NumSymbols;
    float unit = m_Angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f - 0.5f / N, 1.0f);

    int idx = (int)(unit * N) % N; // �؂�̂�
    m_StopIndex = idx;

    // �s�^�b�ƒ���
    float snapAngle = (2.0f * DirectX::XM_PI) * (idx + 0.5f) / N;
    m_Angle = snapAngle;
    m_IsSpinning = false;

    // �f�o�b�O
    char buf[128];
    sprintf_s(buf, "[DEBUG] idx=%d symbol=%s angle=%.3f\n", idx, m_Symbols[idx].c_str(), m_Angle);
    OutputDebugStringA(buf);
}



/// <summary>
/// ���݂̊p�x����o�ڃC���f�b�N�X�ƃV���{���𔻒肷��
/// </summary>
void ReelComponent::JudgeSymbol() {
    float unit = m_Angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f, 1.0f);
    int idx = (int)(unit * s_NumSymbols + 0.5f) % s_NumSymbols;
    m_StopIndex = idx;
    std::string symbol = m_Symbols[m_StopIndex];

    char buf[128];
    sprintf_s(buf, "[DEBUG][JUDGE] idx=%d symbol=%s\n", m_StopIndex, symbol.c_str());
    OutputDebugStringA(buf);
}
