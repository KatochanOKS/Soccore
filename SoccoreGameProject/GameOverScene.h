#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include "GameObject.h"
#include "UIImage.h"
#include <vector>

/// <summary>
/// ゲームオーバー画面のシーン。UIやオブジェクトの初期化・管理・描画・更新を担当する。
/// </summary>
class GameOverScene : public Scene {
public:
    /// <summary>
    /// ゲームオーバーシーンのコンストラクタ
    /// </summary>
    
    explicit GameOverScene(EngineManager* engine);

    /// <summary>
    /// ゲームオーバーシーンのデストラクタ
    /// </summary>
    
    ~GameOverScene() override;

    /// <summary>
    /// シーン開始時の初期化処理
    /// </summary>
    
    void Start() override;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    
    void Update() override;

    /// <summary>
    /// 毎フレームの描画処理
    /// </summary>
    
    void Draw() override;

private:
    EngineManager* engine = nullptr; ///< エンジン管理
    std::vector<GameObject*> m_SceneObjects; ///< シーン内のGameObjectリスト
};