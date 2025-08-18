#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include <vector>
#include <string>

class GameScene : public Scene {
public:
    GameScene(EngineManager* engine);
    ~GameScene() override;
    void Start() override;
    void Update() override;
    void Draw() override;

    // �ǉ�: �^�O�E���O�Ō�������֐�
    GameObject* FindByTag(const std::string& tag);
    GameObject* FindByName(const std::string& name);

private:
    EngineManager* engine = nullptr;
    std::vector<GameObject*> m_sceneObjects;
    DirectX::XMFLOAT3 m_ballVelocity = { 0, 0, 0 };
};
