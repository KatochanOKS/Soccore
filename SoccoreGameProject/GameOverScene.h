#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include "GameObject.h"
#include "UIImage.h"
#include <vector>

class GameOverScene : public Scene {
public:
    explicit GameOverScene(EngineManager* engine);
    ~GameOverScene() override;

    void Start() override;
    void Update() override;
    void Draw() override;

private:
    EngineManager* engine = nullptr;
    std::vector<GameObject*> m_sceneObjects;
};
