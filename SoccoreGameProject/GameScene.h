#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include "ReelComponent.h"
#include "GameUIManager.h"
#include "PlayerManager.h"
#include "AnimationManager.h"
#include "StageManager.h"
#include <vector>
#include <string>
#include <DirectXMath.h>

/// <summary>
/// ゲームのメインシーン。ステージ・UI・プレイヤー・アニメーションなどの初期化・管理・描画・更新を担当する。
/// </summary>
class GameScene : public Scene {
public:
    /// <summary>
    /// ゲームシーンのコンストラクタ
    /// </summary>
    
    GameScene(EngineManager* engine);

    /// <summary>
    /// ゲームシーンのデストラクタ
    /// </summary>
    
    ~GameScene() override;

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

    /// <summary>
    /// 指定したタグを持つGameObjectを検索する
    /// </summary>
    
    GameObject* FindByTag(const std::string& tag);

    /// <summary>
    /// 指定した名前を持つGameObjectを検索する
    /// </summary>
    
    GameObject* FindByName(const std::string& name);

    std::vector<GameObject*> m_SceneObjects; ///< シーン内の全GameObjectリスト

private:

    /// <summary>
    /// アニメーションの登録処理
    /// </summary>
    
    void RegisterAnimations();

    EngineManager* engine = nullptr; ///< エンジン管理
	GameUIManager m_UIManager;    ///< UI管理
	PlayerManager m_PlayerManager;   ///< プレイヤー管理
	AnimationManager m_AnimationManager; ///< アニメーション管理
	StageManager m_StageManage; ///< ステージ管理


    bool IsP1DyingEnded = false;   ///< プレイヤー1死亡演出終了フラグ
    bool IsP2DyingEnded = false;   ///< プレイヤー2死亡演出終了フラグ
};