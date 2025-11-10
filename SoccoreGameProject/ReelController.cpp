#include "ReelController.h"
#include "ReelComponent.h"
#include "ReelJudge.h"
#include "GameScene.h"
#include <Windows.h>
#include <cstdlib>
#include <string>
#include <array>
#include <vector>
#include <algorithm>

void ReelController::SetReels(ReelComponent* left, ReelComponent* middle, ReelComponent* right) {
    m_Left = left; m_Middle = middle; m_Right = right;
}

static bool IsDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

// ===== ユーティリティ：1/N 抽選（N分の1でtrue） =====
static bool Roll1in(int denom) {
    if (denom <= 1) return true;
    // rand() は簡易で十分（必要なら mt19937 に差し替え可）
    int r = std::rand() % denom; // 0 .. denom-1
    return (r == 0);
}

// ===== リール面に存在する図柄（固定名） =====
// ※ あなたのリール配列: {"BAR","力","ベル","リプレイ","7","力","リプレイ","ベル"}
//   → 以下の一覧と一致する名称を使います。
static const std::array<const char*, 5> kSymbols = { "ベル", "リプレイ", "7", "BAR", "力" };

// ===== ハズレ用：基準図柄とは異なる図柄を1つ返す =====
static std::string PickDifferentSymbol(const std::string& base) {
    std::vector<std::string> pool;
    pool.reserve(kSymbols.size()); 
    for (auto s : kSymbols) if (base != s) pool.emplace_back(s);
    if (pool.empty()) return base; // 念のため
    int idx = std::rand() % static_cast<int>(pool.size());
    return pool[idx];
}

// ===== 任意：ハズレ時に使う“見せ方”パターン（ここで揃わない形にする） =====
static void PlanLosePattern(ReelComponent* L, ReelComponent* M, ReelComponent* R, const std::string& base) {
    // 例：L=base, M=異なる図柄, R=base（「ニ連＋スカし」の基本型）
    const std::string other = PickDifferentSymbol(base);
    if (L) L->PlanStopSymbol(base);
    if (M) M->PlanStopSymbol(other);
    if (R) R->PlanStopSymbol(base);
}

// ===== 当選時：3本同じ図柄で予約 =====
static void PlanWinAllSame(ReelComponent* L, ReelComponent* M, ReelComponent* R, const std::string& symbol) {
    if (L) L->PlanStopSymbol(symbol);
    if (M) M->PlanStopSymbol(symbol);
    if (R) R->PlanStopSymbol(symbol);
}

void ReelController::Update() {
    bool z = IsDown('Z');
    bool x = IsDown('X');
    bool c = IsDown('C');
    bool s = IsDown('S');

    // === 停止（立ち上がりのみ） ===
    if (z && !m_IsPrevZ && m_Left)   m_Left->RequestStop();
    if (x && !m_IsPrevX && m_Middle) m_Middle->RequestStop();
    if (c && !m_IsPrevC && m_Right)  m_Right->RequestStop();

    // === スタート（立ち上がりのみ）===
    if (s && !m_IsPrevS) {

        // ------------------------------------------------------------
        // 1) 排他的な当選抽選：上から順に「当たったら即採用」
        //    例）7(1/80) → BAR(1/120) → 力(1/60) → リプレイ(1/7) → ベル(1/2)
        //    ※ 上にあるほど優先度が高い。二重当選は起こさない。
        // ------------------------------------------------------------
        struct Entry { const char* symbol; int denom; };
        // ← 好きに調整OK（“1/denom” が当選確率）
        const std::array<Entry, 5> table = { {
            { "7",        80  },   // 1/80
            { "BAR",      120 },   // 1/120
            { "力",       60  },   // 1/60
            { "リプレイ", 7   },   // 1/7
            { "ベル",     2   },   // 1/2（体験用に高め）
        } };

        std::string outcome;   // 当選した図柄名（空ならハズレ）
        for (const auto& e : table) {
            if (Roll1in(e.denom)) { outcome = e.symbol; break; }
        }

        // ------------------------------------------------------------
        // 2) 当選/ハズレで“出目予約”を分岐
        // ------------------------------------------------------------
        if (!outcome.empty()) {
            // 当選：3本すべて同じ図柄を予約（＝揃う）
            PlanWinAllSame(m_Left, m_Middle, m_Right, outcome);
        }
        else {
            // ハズレ：意図的に揃えない形を作る
            const std::string base = "ベル";
            PlanLosePattern(m_Left, m_Middle, m_Right, base);
        }

        // ------------------------------------------------------------
        // 3) 予約してから回転開始
        // ------------------------------------------------------------
        if (m_Left)   m_Left->RequestStart();
        if (m_Middle) m_Middle->RequestStart();
        if (m_Right)  m_Right->RequestStart();

        m_ResultShown = false; // スタート時にリセット
    }


    m_IsPrevZ = z; m_IsPrevX = x; m_IsPrevC = c; m_IsPrevS = s;

    // === 全リール停止後の出目判定 ===
    if (m_Left && m_Middle && m_Right)
    {
        if (!m_Left->IsSpinning() && !m_Middle->IsSpinning() && !m_Right->IsSpinning())
        {
            if (!m_ResultShown) { // 一度だけ表示
                std::array<std::string, 3> symbols = {
                    m_Left->GetCurrentSymbol(),
                    m_Middle->GetCurrentSymbol(),
                    m_Right->GetCurrentSymbol()
                };
                std::string result = ReelJudge::Judge(symbols);

                OutputDebugStringA(("出目結果: " + result + "\n").c_str());

                // ★ ここでシーンへ通知して、小役ごとの処理を実行
                if (gameObject && gameObject->scene) {
                    if (auto* gs = dynamic_cast<GameScene*>(gameObject->scene)) {
                        gs->ApplySlotEffect(result);
                    }
                }

                m_ResultShown = true; // 表示済みに
            }
        }
    }


}
