// UIComponent.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

/// <summary>
/// UI要素の位置・サイズ・表示状態などを管理する基本コンポーネント
/// </summary>

class UIComponent : public Component {
public:
    DirectX::XMFLOAT2 m_Position{ 0, 0 };   ///< 画面座標 (ピクセル)
    DirectX::XMFLOAT2 m_Size{ 100, 100 };   ///< UIサイズ
    float m_Layer = 0.0f;                   ///< 重なり順（レイヤー値）
    bool m_Visible = true;                  ///< 表示状態

    /// <summary>
    /// UI描画処理（Renderer経由で呼び出し）
    /// </summary>
    
    virtual void Draw(class Renderer* renderer, size_t idx) {}

    /// <summary>
    /// UI状態の毎フレーム更新
    /// </summary>
    
    virtual void Update(float dt) {}
};