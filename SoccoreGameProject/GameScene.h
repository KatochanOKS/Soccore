#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include <vector>

class GameScene : public Scene {
public:
    GameScene(EngineManager* engine);

    void Start() override;
    void Update() override;
    void Draw() override;

private:
    EngineManager* engine = nullptr;
    std::vector<GameObject*> m_sceneObjects; // Å© Ç±Ç±ÇæÇØÇ≈ä«óù
};
