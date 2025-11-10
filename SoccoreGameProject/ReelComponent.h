#pragma once
#include "Component.h"
#include <unordered_map>
#include <random>
#include <string>
#include <vector>

/// <summary>
/// スロットリールの回転・停止・出目判定を管理するコンポーネント。
/// </summary>
/// <remarks>
/// ・8コマの円柱リールを回転させ、中心線通過タイミングで停止コマを決定する。<br>
/// ・1コマ目の中心は22.5°、以降45°刻みで各コマ中心が配置される。<br>
/// ・TransformのX軸回転値に角度を反映し、回転を制御する。<br>
/// ・入力は持たず、外部（ReelController等）から RequestStart/RequestStop を受ける。<br>
/// </remarks>
class ReelComponent : public Component {
public:
    /// <summary>コンストラクタ。デフォルト図柄名（7/BAR/フルーツ等）を設定する。</summary>
    ReelComponent();

    /// <summary>毎フレーム更新。回転→（停止リクエスト時に）停止先決定→吸着→Transform反映。</summary>
    void Update() override;

    /// <summary>回転速度を設定（再開時の既定速度も同時に更新）。負で時計回り。</summary>
    void SetSpeed(float speed) { m_Speed = speed; m_DefaultSpeed = speed; }

    /// <summary>図柄名リストを設定（ログ/UI表示用）。</summary>
    void SetSymbols(const std::vector<std::string>& symbols);

    /// <summary>停止した（または停止予定の）コマのインデックスを取得。</summary>
    int GetStopIndex() const { return m_StopIndex; }

    /// <summary>現在リールが回転中か。</summary>
    bool IsSpinning() const { return m_IsSpinning; }

    /// <summary>回転開始（既定速度で再開）。</summary>
    void RequestStart();

    /// <summary>停止要求（次フレームで「今or次」判定→吸着停止）。</summary>
    void RequestStop();

    /// <summary>役ごとの確率を設定（例：{"ベル":0.15f,"リプレイ":0.2f,"7":0.01f}）。合計が1未満なら不足分はハズレへ。</summary>
    void SetCategoryProbabilities(const std::unordered_map<std::string, float>& table);

    /// <summary>ハズレ役の名前を変更（デフォルト "ハズレ"）。</summary>
    void SetFallbackSymbol(const std::string& name) { m_FallbackSymbol = name; }

    /// <summary>次回停止時に狙う図柄を予約（コントローラからの指示）。停止決定時に自動消費。</summary>
    void PlanStopSymbol(const std::string& symbol);

    std::string GetCurrentSymbol() const;


private:
    // ===== 数学ユーティリティ =====
    static float Wrap01(float x);          // 値を0〜1に正規化
    static float Wrap2PI(float a);         // 角度を0〜2πに正規化
    float AngleToUnit(float angleRad) const;     // 角度(rad)→単位角度(0..1)
    int   UnitToCenterIndex(float unit) const;   // 単位角度→“正面に最も近いコマ”index
    float IndexToCenterAngle(int idx) const;     // index→コマ中心角(rad)
    int   DecideStopIndex(float prevAngle, float nowAngle) const; // 今回は未使用でも残す

    // ===== 停止決定補助 =====
    std::string WeightedPickCategory();
    int   FindNextIndex(const std::string& symbol, int fromIdx) const;
    int   CurrentCenterIndex() const;

private:
    // ===== 定数設定 =====
    const int   m_Num = 8;                    ///< コマ数
    const float m_StepDeg = 360.0f / 8.0f;    ///< コマ1つの角度（45°）
    const float m_FirstCenterDeg = 22.5f;     ///< 1コマ目の中心角
    const float m_ZeroDegDeg = 0.0f;          ///< 中心線（正面）の角度（0°）

    // ラジアン変換済み定数
    const float m_StepRad = m_StepDeg * 3.1415926535f / 180.0f;
    const float m_FirstCenterRad = m_FirstCenterDeg * 3.1415926535f / 180.0f;
    const float m_ZeroDegRad = m_ZeroDegDeg * 3.1415926535f / 180.0f;
    const float m_TurnRad = 6.28318530718f;  ///< 2π

    // ===== 実行時状態 =====
    float m_Speed = -0.012f;  ///< 現在角速度（負で時計回り）
    float m_DefaultSpeed = -0.012f;  ///< 再開時の既定速度
    float m_Angle = 0.0f;     ///< 現在回転角（rad）
    bool  m_IsSpinning = false;    ///< 回転中
    bool  m_StopRequested = false;    ///< 停止要求
    bool  m_IsDecel = false;    ///< 減速/吸着中
    int   m_StopIndex = -1;       ///< 停止対象index
    float m_TargetAngle = 0.0f;     ///< 停止ターゲット角

    std::vector<std::string> m_Symbols; ///< 図柄名（ログ/UI用）

    // 役確率テーブル
    std::unordered_map<std::string, float> m_CategoryProb;
    std::string m_FallbackSymbol = "ハズレ";

    // 乱数
    std::mt19937 m_Rng{ 0xC0FFEE };

    // 次回停止予約
    std::string m_PlannedSymbol;
};
