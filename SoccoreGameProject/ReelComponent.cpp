#include "ReelComponent.h"
#include "Transform.h"
#include "GameObject.h"
#include <Windows.h>
#include <DirectXMath.h>
#include <cmath>
#include <cstdio>

// 回転・停止処理
void ReelComponent::Update() {
    if (isSpinning) {
        angle += speed;

        // 2πループ
        if (angle > DirectX::XM_PI * 2.0f) angle -= DirectX::XM_PI * 2.0f;
        if (angle < 0) angle += DirectX::XM_PI * 2.0f;

        // 停止ボタン押下
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            isStopping = true;
        }

        if (isStopping) {
            // コマの中心到達判定
            const int N = numSymbols;
            float unitPrev = fmodf((angle - speed) / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f); // 前フレーム
            float unitNow = fmodf(angle / (2.0f * DirectX::XM_PI) + 1.0f, 1.0f);

            // 「-0.5f/N」で中央基準に
            unitPrev = fmodf(unitPrev - 0.5f / N + 1.0f, 1.0f);
            unitNow = fmodf(unitNow - 0.5f / N + 1.0f, 1.0f);

            int idxPrev = (int)(unitPrev * N);
            int idxNow = (int)(unitNow * N);

            if (idxPrev != idxNow) {
                // コマの中心を通過したタイミング！
                StopAndSnap(); // ピタッとコマ中心へスナップ＆出目判定
                isStopping = false;
                isSpinning = false;
            }
        }
    }
    // Transform反映
    if (auto* tr = gameObject->GetComponent<Transform>()) {
        tr->rotation.x = angle;
    }
}

void ReelComponent::StopAndSnap() {
    int N = numSymbols;
    float unit = angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f - 0.5f / N, 1.0f);

    int idx = (int)(unit * N) % N; // 切り捨て
    stopIndex = idx;

    // ピタッと中央
    float snapAngle = (2.0f * DirectX::XM_PI) * (idx + 0.5f) / N;
    angle = snapAngle;
    isSpinning = false;

    // デバッグ
    char buf[128];
    sprintf_s(buf, "[DEBUG] idx=%d symbol=%s angle=%.3f\n", idx, symbols[idx].c_str(), angle);
    OutputDebugStringA(buf);
}



// ---- 「出目のみ判定したい」場合にも対応 ----
void ReelComponent::JudgeSymbol() {
    // 今のangleからidx計算（StopAndSnapでも同じ）
    float unit = angle / (2.0f * DirectX::XM_PI);
    unit = fmodf(unit + 1.0f, 1.0f);
    int idx = (int)(unit * numSymbols + 0.5f) % numSymbols;
    stopIndex = idx;
    std::string symbol = symbols[stopIndex];

    char buf[128];
    sprintf_s(buf, "[DEBUG][JUDGE] idx=%d symbol=%s\n", stopIndex, symbol.c_str());
    OutputDebugStringA(buf);
}
