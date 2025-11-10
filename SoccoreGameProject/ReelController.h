#pragma once
#include "Component.h"

class ReelComponent;

/// <summary>
/// 3本のリールに対する入力受付と命令発行を行うコントローラ。
/// 既定：Z=左停止、X=中停止、C=右停止、S=3本同時スタート。
/// </summary>
class ReelController : public Component {
public:
    /// <summary>各リール（左・中・右）を登録する。</summary>
    void SetReels(ReelComponent* left, ReelComponent* middle, ReelComponent* right);

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
    bool m_ResultShown = false;
};
