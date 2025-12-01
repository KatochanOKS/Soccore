#pragma once
#include "Component.h"

class ReelComponent;

enum class SlotOwner {
    Player1,
    Player2
};

/// <summary>
/// 3本のリールに対する入力受付と命令発行を行うコントローラ。
/// 既定：Z=左停止、X=中停止、C=右停止、S=3本同時スタート。
/// </summary>
class ReelController : public Component {
public:
    /// <summary>各リール（左・中・右）を登録する。</summary>
    void SetReels(ReelComponent* left, ReelComponent* middle, ReelComponent* right);

    void SetOwner(SlotOwner owner) {
        m_Owner = owner;
    }

    /// <summary>毎フレーム更新。キー入力を見て各リールへ命令を出す。</summary>
    void Update() override;

private:
    ReelComponent* m_Left = nullptr;
    ReelComponent* m_Middle = nullptr;
    ReelComponent* m_Right = nullptr;

    bool m_IsPrevZ = false;
    bool m_IsPrevX = false;
    bool m_IsPrevC = false;
    bool m_IsPrevS = false;
	bool m_ResultShown = false; // 結果表示済みフラグ
    bool m_IsStarted = false; // 一度でも回転したら true
	bool m_CanStart = true; // スタート可能フラグ

	float m_Timer = 0.0f; //経過時間

    SlotOwner m_Owner = SlotOwner::Player1;
};
