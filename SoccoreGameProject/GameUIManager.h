#pragma once
#include <vector>
class EngineManager;
class GameObject;

/// <summary>
/// UI�v�f�̐����E�Ǘ���S������N���X
/// </summary>
class GameUIManager {
public:
    void InitUI(EngineManager* engine, std::vector<GameObject*>& sceneObjects);
};