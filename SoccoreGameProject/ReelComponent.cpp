#include "ReelComponent.h"
#include "Transform.h"
#include "GameObject.h"
#include <Windows.h>
#include <DirectXMath.h>
#include <cmath>
#include <cstdio>

// ��]�E��~����
void ReelComponent::Update() {
    if (isSpinning) {
        angle += speed;

        // 2�΃��[�v
        if (angle > DirectX::XM_PI * 2.0f) angle -= DirectX::XM_PI * 2.0f;
        if (angle < 0) angle += DirectX::XM_PI * 2.0f;

        // ��~�{�^������
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            isStopping = true;
        }

        if (isStopping) {
            // �R�}�̒��S���B����
            const int N = numSymbols;
            float unitPrev = fmodf((angle - speed) / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f); // �O�t���[��
            float unitNow = fmodf(angle / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f);

            // �u-0.5f/N�v�Œ������
            unitPrev = fmodf(unitPrev - 0.5f / N + 1.0f, 1.0f);
            unitNow = fmodf(unitNow - 0.5f / N + 1.0f, 1.0f);

            int idxPrev = (int)(unitPrev * N);
            int idxNow = (int)(unitNow * N);

            if (idxPrev != idxNow) {
                // �R�}�̒��S��ʉ߂����^�C�~���O�I
                StopAndSnap(); // �s�^�b�ƃR�}���S�փX�i�b�v���o�ڔ���
                isStopping = false;
                isSpinning = false;
            }
        }
    }
    // Transform���f
    if (auto* tr = gameObject->GetComponent<Transform>()) {
        tr->rotation.x = angle;
    }
}

void ReelComponent::StopAndSnap() {
    int N = numSymbols;
    float unit = angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f - 0.5f / N, 1.0f);

    int idx = (int)(unit * N) % N; // �؂�̂�
    stopIndex = idx;

    // �s�^�b�ƒ���
    float snapAngle = (2.0f * DirectX::XM_PI) * (idx + 0.5f) / N;
    angle = snapAngle;
    isSpinning = false;

    // �f�o�b�O
    char buf[128];
    sprintf_s(buf, "[DEBUG] idx=%d symbol=%s angle=%.3f\n", idx, symbols[idx].c_str(), angle);
    OutputDebugStringA(buf);
}



// ---- �u�o�ڂ̂ݔ��肵�����v�ꍇ�ɂ��Ή� ----
void ReelComponent::JudgeSymbol() {
    // ����angle����idx�v�Z�iStopAndSnap�ł������j
    float unit = angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f, 1.0f);
    int idx = (int)(unit * numSymbols + 0.5f) % numSymbols;
    stopIndex = idx;
    std::string symbol = symbols[stopIndex];

    char buf[128];
    sprintf_s(buf, "[DEBUG][JUDGE] idx=%d symbol=%s\n", stopIndex, symbol.c_str());
    OutputDebugStringA(buf);
}
