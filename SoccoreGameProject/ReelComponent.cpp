#include "ReelComponent.h"
#include "Transform.h"
#include "GameObject.h"
#include <Windows.h>
#include <DirectXMath.h>
#include <cmath>
#include <cstdio>

/// <summary>
/// リールの回転・停止処理を毎フレーム更新する
/// </summary>
void ReelComponent::Update() {
    if (m_IsSpinning) {
        m_Angle += m_Speed;

        // 2πループ
        if (m_Angle > DirectX::XM_PI * 2.0f) m_Angle -= DirectX::XM_PI * 2.0f;
        if (m_Angle < 0) m_Angle += DirectX::XM_PI * 2.0f;

        // 停止ボタン押下
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            m_IsStopping = true;
        }

        if (m_IsStopping) {
            // コマの中心到達判定
            const int N = s_NumSymbols;
            float unitPrev = fmodf((m_Angle - m_Speed) / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f); // 前フレーム
            float unitNow = fmodf(m_Angle / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f);

            // 「-0.5f/N」で中央基準に
            unitPrev = fmodf(unitPrev - 0.5f / N + 1.0f, 1.0f);
            unitNow = fmodf(unitNow - 0.5f / N + 1.0f, 1.0f);

            int idxPrev = (int)(unitPrev * N);
            int idxNow = (int)(unitNow * N);

            if (idxPrev != idxNow) {
                // コマの中心を通過したタイミング！
                StopAndSnap(); // ピタッとコマ中心へスナップ＆出目判定
                m_IsStopping = false;
                m_IsSpinning = false;
            }
        }
    }
    // Transform反映
    if (auto* tr = gameObject->GetComponent<Transform>()) {
        tr->rotation.x = m_Angle;
    }
}

/// <summary>
/// コマの中心に角度をスナップし、停止インデックスと出目を確定する
/// </summary>
void ReelComponent::StopAndSnap() {
    int N = s_NumSymbols;
    float unit = m_Angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f - 0.5f / N, 1.0f);

    int idx = (int)(unit * N) % N; // 切り捨て
    m_StopIndex = idx;

    // ピタッと中央
    float snapAngle = (2.0f * DirectX::XM_PI) * (idx + 0.5f) / N;
    m_Angle = snapAngle;
    m_IsSpinning = false;

    // デバッグ
    char buf[128];
    sprintf_s(buf, "[DEBUG] idx=%d symbol=%s angle=%.3f\n", idx, m_Symbols[idx].c_str(), m_Angle);
    OutputDebugStringA(buf);
}



/// <summary>
/// 現在の角度から出目インデックスとシンボルを判定する
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
