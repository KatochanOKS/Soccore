#include "StageManager.h"
#include "ObjectFactory.h"
#include "Colors.h"

/// <summary>
/// ステージマネージャのコンストラクタ
/// </summary>

StageManager::StageManager(EngineManager* engine)
    : engine(engine) {}

/// <summary>
/// ステージマネージャのデストラクタ
/// </summary>

StageManager::~StageManager() {}

/// <summary>
/// ステージの初期化処理
/// </summary>

void StageManager::InitStage(std::vector<GameObject*>& sceneObjects) {
    int groundTex = engine->GetTextureManager()->LoadTexture(
        L"assets/Floor.png",
        engine->GetDeviceManager()->GetCommandList()
    );

    sceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0.0f, -0.5f, 0.0f }, { 10.0f, 1.0f, 3.0f },
        groundTex,
        Colors::Gray, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    int reelTex = engine->GetTextureManager()->LoadTexture(
        L"assets/Slot/Reel.png",
        engine->GetDeviceManager()->GetCommandList()
    );

    // --- ★1P用リール ---
    ReelComponent* reelsP1[3] = { nullptr, nullptr, nullptr };
    for (int i = 0; i < 3; ++i) {
        const float x = -5.0f + i * 1.5f;   // 左側に表示

        GameObject* reel = ObjectFactory::CreateCylinderReel(
            engine,
            { x, 3.0f, 0.0f },
            { 1.5f, 1.5f, 1.5f },
            reelTex,
            Colors::White,
            "ReelP1",
            "P1_SlotReel" + std::to_string(i + 1)
        );
        sceneObjects.push_back(reel);
        auto* rc = reel->GetComponent<ReelComponent>();
        rc->SetSpeed(-0.04f);
        reelsP1[i] = rc;
    }

    {
        GameObject* controller = new GameObject();
        controller->tag = "ReelControllerP1";
        controller->name = "ReelControllerP1";

        auto* ctrl = controller->AddComponent<ReelController>();
        ctrl->SetReels(reelsP1[0], reelsP1[1], reelsP1[2]);
        ctrl->SetOwner(SlotOwner::Player1);   // ★ここ重要

        sceneObjects.push_back(controller);
    }

    // --- ★2P用リール ---
    ReelComponent* reelsP2[3] = { nullptr, nullptr, nullptr };
    for (int i = 0; i < 3; ++i) {
        const float x = +2.0f + i * 1.5f;   // 右側に表示（位置はお好みで）

        GameObject* reel = ObjectFactory::CreateCylinderReel(
            engine,
            { x, 3.0f, 0.0f },
            { 1.5f, 1.5f, 1.5f },
            reelTex,
            Colors::White,
            "ReelP2",
            "P2_SlotReel" + std::to_string(i + 1)
        );
        sceneObjects.push_back(reel);
        auto* rc = reel->GetComponent<ReelComponent>();
        rc->SetSpeed(-0.04f);
        reelsP2[i] = rc;
    }

    {
        GameObject* controller = new GameObject();
        controller->tag = "ReelControllerP2";
        controller->name = "ReelControllerP2";

        auto* ctrl = controller->AddComponent<ReelController>();
        ctrl->SetReels(reelsP2[0], reelsP2[1], reelsP2[2]);
        ctrl->SetOwner(SlotOwner::Player2);   // ★2P用と指定

        sceneObjects.push_back(controller);
    }
}
