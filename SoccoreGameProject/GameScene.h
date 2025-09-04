#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include <vector>
#include <string>
#include <DirectXMath.h>

class GameScene : public Scene {
public:
    GameScene(EngineManager* engine);
    ~GameScene() override;
    void Start() override;
    void Update() override;
    void Draw() override;

    GameObject* FindByTag(const std::string& tag);
    GameObject* FindByName(const std::string& name);

    std::vector<GameObject*> m_sceneObjects;
private:
    EngineManager* engine = nullptr;

    bool p1DyingEnded = false;
    bool p2DyingEnded = false;
};
