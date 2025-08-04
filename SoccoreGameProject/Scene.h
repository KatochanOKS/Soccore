// Scene.h
#pragma once

// すべてのシーンの共通インターフェース
class Scene {
public:
    virtual ~Scene() {}
    // 初期化（最初の1回だけ呼ばれる）
    virtual void Start() = 0;
    // 毎フレーム呼ばれる
    virtual void Update() = 0;
    // 毎フレーム呼ばれる（描画処理）
    virtual void Draw() = 0;
};
