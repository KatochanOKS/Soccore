#pragma once
#include <vector>
class EngineManager;
class GameObject;

/// <summary>
/// UI要素の生成・管理を担当するクラス
/// </summary>
class GameUIManager {
public:
    void InitUI(EngineManager* engine, std::vector<GameObject*>& sceneObjects);
};