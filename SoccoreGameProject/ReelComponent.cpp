#include "ReelComponent.h"
#include "Transform.h"
#include "GameObject.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <DirectXMath.h>

using namespace DirectX;

// ===== コンストラクタ =====
ReelComponent::ReelComponent()
    : m_Symbols({ "BAR", "力", "ベル", "リプレイ", "7", "力", "リプレイ", "ベル" })
{
}

// ===== 図柄設定 =====
void ReelComponent::SetSymbols(const std::vector<std::string>& symbols) {
    m_Symbols = symbols;
}

// ===== 数学ユーティリティ =====
float ReelComponent::Wrap01(float x) {
    x = fmodf(x, 1.0f);
    return (x < 0) ? (x + 1.0f) : x;
}
// 2πに正規化
float ReelComponent::Wrap2PI(float a) {
    const float T = 6.28318530718f; // 2π
    a = fmodf(a, T);
    return (a < 0) ? (a + T) : a;
}
// 角度(rad)→単位角度(0..1)
float ReelComponent::AngleToUnit(float angleRad) const {
    return Wrap01((angleRad - m_ZeroDegRad) / m_TurnRad);
}

int ReelComponent::UnitToCenterIndex(float unit) const {
    // 1コマ目中心(22.5°)を原点に合わせるため左シフト
    float u = Wrap01(unit - (m_FirstCenterRad / m_TurnRad));
    int idx = int(floorf(u * m_Num)); // 0〜7
    return idx % m_Num;
}

float ReelComponent::IndexToCenterAngle(int idx) const {
    return Wrap2PI(m_FirstCenterRad + idx * m_StepRad);
}

int ReelComponent::DecideStopIndex(float prevAngle, float nowAngle) const {
    float uPrev = AngleToUnit(prevAngle);   // 前回角度を「0～1の単位角度」に変換
    float uNow = AngleToUnit(nowAngle);    // 現在角度を「0～1の単位角度」に変換
    int idxPrev = UnitToCenterIndex(uPrev); // 前回角度が指す“正面に最も近いコマ”のインデックス取得
    int idxNow = UnitToCenterIndex(uNow);  // 現在角度が指す“正面に最も近いコマ”のインデックス取得

    if (idxNow == idxPrev) {
        return idxNow; // 未通過 → 今のコマ
    }
    else {
        return (idxNow + 1) % m_Num; // 通過済み → 次のコマ（時計回り）
    }
}

// ===== メイン更新 =====
void ReelComponent::Update() {
    float prevAngle = m_Angle;

    // 回転中（減速中でない）なら角度を進める
    if (m_IsSpinning && !m_IsDecel) {
        m_Angle = Wrap2PI(m_Angle + m_Speed);
    }

    // ==== 停止要求を消費して停止先を決める ====
    if (m_StopRequested) {
        // 1) 予約があれば最優先。無ければ従来の確率抽選。
        std::string role;
        if (!m_PlannedSymbol.empty()) {
            role = m_PlannedSymbol;
        }
        else {
            role = WeightedPickCategory(); // 既存ロジック流用
        }

        // 2) 現在の“正面に最も近いコマ”から、時計回りで次に現れる一致図柄を探す
        const int curIdx = CurrentCenterIndex();
        int targetIdx = FindNextIndex(role, curIdx);

        // 3) 見つからなければフォールバック → それも無ければ現位置
        if (targetIdx < 0) {
            targetIdx = FindNextIndex(m_FallbackSymbol, curIdx);
            if (targetIdx < 0) targetIdx = curIdx;
        }

        m_StopIndex = targetIdx;
        m_TargetAngle = IndexToCenterAngle(targetIdx);

        // 次のフェーズへ
        m_IsDecel = true;     // 減速/吸着へ
        m_StopRequested = false;    // 消費
        m_PlannedSymbol.clear();    // 予約消費
    }
    // ---- 減速・吸着フェーズ ----
    else if (m_IsDecel) {
        // 新コード（時計回りのみで寄せる）
        const float TWO_PI = 6.28318530718f;

        // 現在角から目標角へ「時計回り（角度を減らす方向）」に進んだときの距離 [0..2π)
        float distCW = fmodf((m_Angle - m_TargetAngle) + TWO_PI, TWO_PI);

        // 十分近ければスナップして停止
        if (distCW < 0.002f) {
            m_Angle = m_TargetAngle;
            m_IsDecel = false;
            m_IsSpinning = false;
        }
        else {
            const float k = 0.25f; // 吸着係数（大→早く止まる）
            float step = distCW * k;

            // 時計回り＝角度を減らす方向にだけ進める
            m_Angle = Wrap2PI(m_Angle - step);
        }
    }

    // ---- Transformへ反映（X軸回転）----
    if (auto* tr = gameObject->GetComponent<Transform>()) {
        tr->rotation.x = m_Angle;
    }
}

// ===== 操作系 =====
void ReelComponent::RequestStart() {
    if (!m_IsSpinning && !m_IsDecel) {
        m_IsSpinning = true;
        m_StopRequested = false;
        m_Speed = m_DefaultSpeed;
    }
}

void ReelComponent::RequestStop() {
    if (m_IsSpinning && !m_IsDecel) {
        m_StopRequested = true;
    }
}

// ===== 役確率設定/抽選 =====
void ReelComponent::SetCategoryProbabilities(const std::unordered_map<std::string, float>& table) {
    m_CategoryProb = table; // 正規化や不足分のハズレ配分は抽選時に行う
}

std::string ReelComponent::WeightedPickCategory() {
    double sum = 0.0;
    for (auto& kv : m_CategoryProb) {
        sum += std::max(0.0f, kv.second);
    }

    std::unordered_map<std::string, double> norm;
    for (auto& kv : m_CategoryProb) {
        norm[kv.first] = std::max(0.0, (double)kv.second);
    }

    if (sum < 1.0 - 1e-6) {
        norm[m_FallbackSymbol] += (1.0 - sum); // 不足分をハズレへ
        sum = 1.0;
    }
    if (sum <= 0.0) {
        return m_FallbackSymbol; // 何もなければハズレ100%
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(m_Rng);
    double acc = 0.0;
    for (auto& kv : norm) {
        acc += kv.second / sum;
        if (r < acc) return kv.first;
    }
    return norm.begin()->first; // 端数誤差フォールバック
}

int ReelComponent::CurrentCenterIndex() const {
    float u = AngleToUnit(m_Angle);
    return UnitToCenterIndex(u);
}

int ReelComponent::FindNextIndex(const std::string& symbol, int fromIdx) const {
    if (m_Symbols.empty()) return -1;
    for (int step = 0; step < (int)m_Symbols.size(); ++step) {
        int idx = (fromIdx + step) % (int)m_Symbols.size();
        if (m_Symbols[idx] == symbol) return idx;
    }
    return -1;
}

void ReelComponent::PlanStopSymbol(const std::string& symbol) {
    m_PlannedSymbol = symbol;
}

std::string ReelComponent::GetCurrentSymbol() const {
    if (m_Symbols.empty()) return "";
    int idx = (m_StopIndex >= 0) ? m_StopIndex : CurrentCenterIndex();
    return m_Symbols[idx % m_Symbols.size()];
}

