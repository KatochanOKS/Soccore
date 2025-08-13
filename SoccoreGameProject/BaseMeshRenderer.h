// BaseMeshRenderer.h
#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Renderer;

//----------------------------------------
// すべてのメッシュレンダラーの基底クラス
// - Cube、FBX、アニメFBXなどを組み合わせられるようにする
// - Renderer::DrawObject などで、端的に Draw()を呼び出す
//----------------------------------------
class BaseMeshRenderer : public Component {
public:
    int texIndex = -1;                              // テクスチャのSRV絵柄番号
    DirectX::XMFLOAT4 color = { 1,1,1,1 };          // 表示色 (RGBA)

    // --- 描画関数 ---
    virtual void Draw(
        Renderer* renderer,               // 描画管理クラス
        size_t objIdx,                   // オブジェクト番号
        const DirectX::XMMATRIX& view,   // ビュー行列
        const DirectX::XMMATRIX& proj    // プロジェクション行列
    ) = 0; // 抽象基底のため pure virtual

    virtual ~BaseMeshRenderer() {}
};
