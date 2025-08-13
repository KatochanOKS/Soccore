#pragma once
#include "Component.h"
#include <DirectXMath.h>

// ------------------------------
// 全てのメッシュレンダラーの「共通部分」だけを持つ親クラス
// ・色やテクスチャ番号など共通データ
// ・Draw関数は必ず子クラスで定義（=0）
// ------------------------------
class BaseMeshRenderer : public Component {
public:
    int texIndex = -1;  // テクスチャインデックス（使わない場合は-1）
    DirectX::XMFLOAT4 color = { 1, 1, 1, 1 }; // 色データ（R,G,B,A）

    // 「描画関数」を仮想関数にしておく（=0は「純粋仮想関数」）
    // → 必ず子クラスで中身を書かないといけない！
    virtual void Draw() = 0;

    // デストラクタ（必ずvirtual! 継承時の破棄漏れバグ防止）
    virtual ~BaseMeshRenderer() {}
};
