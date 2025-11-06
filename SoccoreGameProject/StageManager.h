#pragma once
#include <vector>
#include "GameObject.h"
#include "EngineManager.h"
#include "ReelComponent.h"
#include "ReelController.h"

/// <summary>
/// ステージ管理クラス。地形やリールなどのステージオブジェクト生成・管理を担当する。
/// </summary> 

class StageManager {
public:
    /// <summary>
    /// ステージマネージャのコンストラクタ
    /// </summary>
    
	StageManager(EngineManager* engine);

    /// <summary>
    /// ステージマネージャのデストラクタ
    /// </summary>
    
    ~StageManager();

    /// <summary>
    /// ステージの初期化処理
    /// </summary>
    
    void InitStage(std::vector<GameObject*>& sceneObjects);

private:
    EngineManager* engine = nullptr; ///< エンジン管理
};