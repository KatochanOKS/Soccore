#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include "GameObject.h"
#include "UIImage.h"
#include <vector>

class StartScene : public Scene {
public:
    StartScene(EngineManager* engine);
    ~StartScene() override;

    void Start() override;
    void Update() override;
    void Draw() override;

private:
    EngineManager* engine = nullptr;
	std::vector<GameObject*> m_sceneObjects;  // シーン内の全オブジェクトを保持
};
