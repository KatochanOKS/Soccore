#pragma once
#include "Component.h"
#include <DirectXMath.h>

/// <summary>
/// 全てのメッシュレンダラーの共通部分を持つ基底クラス。
/// 色やテクスチャインデックスなどの共通データを管理し、
/// Draw関数は必ず子クラスで実装する。
/// </summary>
class BaseMeshRenderer : public Component {
public:
    int m_TexIndex = -1;                        ///< テクスチャインデックス（未使用時は-1）
    DirectX::XMFLOAT4 m_Color = { 1, 1, 1, 1 }; ///< 色データ（R,G,B,A）

    /// <summary>
    /// 描画処理（純粋仮想関数）
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// デストラクタ（必ずvirtual! 継承時の破棄漏れバグ防止）
    /// </summary>
    virtual ~BaseMeshRenderer() {}
};