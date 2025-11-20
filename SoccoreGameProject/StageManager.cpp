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
    int groundTex = engine->GetTextureManager()->LoadTexture(L"assets/Floor.png", engine->GetDeviceManager()->GetCommandList());

    sceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0.0f, -0.5f, 0.0f }, { 10.0f, 1.0f, 3.0f },
        groundTex,
        Colors::Gray, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    int reelTex = engine->GetTextureManager()->LoadTexture(L"assets/Slot/Reel.png", engine->GetDeviceManager()->GetCommandList());

    // --- スロットリール3本を横並びで生成＆コントローラ配線 ---
    ReelComponent* reels[3] = { nullptr, nullptr, nullptr };

    for (int i = 0; i < 3; ++i) {
        const float x = -5.0f + i * 1.5f;

        GameObject* reel = ObjectFactory::CreateCylinderReel(
            engine,
            { x, 3.0f, 0.0f },          // 位置
            { 1.5f, 1.5f, 1.5f },       // スケール
            reelTex,                    // テクスチャ
            Colors::White,              // 色
            "Reel",                     // Tag
            "SlotReel" + std::to_string(i + 1) // Name
        );
        sceneObjects.push_back(reel);
        auto* rc = reel->GetComponent<ReelComponent>();
        rc->SetSpeed(-0.04f);
        reels[i] = rc;
    }

    // 入力→各リールへ命令を出すコントローラ（Z=左, X=中, C=右, S=全スタート）
    {
        GameObject* controller = new GameObject();
        controller->tag = "ReelController";
        controller->name = "ReelController";

        auto* ctrl = controller->AddComponent<ReelController>();
        ctrl->SetReels(reels[0], reels[1], reels[2]);

        sceneObjects.push_back(controller);
    }
}