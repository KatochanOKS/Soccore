// UIImage.h
#pragma once
#include "UIComponent.h"

/// <summary>
/// 画像表示専用のUIコンポーネント（テクスチャID・色付き）
/// </summary>

class UIImage : public UIComponent {
public:
    int m_TexIndex = -1;                    ///< 使用するテクスチャのインデックス
    DirectX::XMFLOAT4 m_Color{ 1, 1, 1, 1 };///< RGBAカラー

    /// <summary>
    /// 画像UIの描画リクエスト（実際の描画処理はRendererで行う）
    /// </summary>
    
    void Draw(class Renderer* renderer, size_t idx) override;
};