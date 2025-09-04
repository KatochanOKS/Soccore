#include "GameOverScene.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Renderer.h"
#include "Camera.h"
#include "StartScene.h"   // タイトルへ戻るため

GameOverScene::GameOverScene(EngineManager* engine) : engine(engine) {}

void GameOverScene::Start() {
    m_sceneObjects.clear();


    // 「GAME OVER」画像（無ければテキスト画像を用意してね）
    int overTex = engine->GetTextureManager()->LoadTexture(
        L"assets/GameOver.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    {
        auto* ui = new GameObject();
        auto* img = ui->AddComponent<UIImage>();
        img->texIndex = overTex;         // 画像が無いときは -1 でもOK（色ベタ表示）
        img->size = { 1280, 720 };
        img->position = { 0, 0 };    // 1280x720想定の中央付近
        img->color = { 1, 1, 1, 1 };
        m_sceneObjects.push_back(ui);
    }
}

void GameOverScene::Update() {
    // エッジ検出（押した瞬間だけ反応）
    static bool prev = false;
    bool cur = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;

    if (cur && !prev) {
        // タイトルへ
        engine->ChangeScene(std::make_unique<StartScene>(engine));
        return;
    }
    prev = cur;

    // 必要があれば点滅などのUI演出をここで
}

void GameOverScene::Draw() {
    auto* cam = engine->GetCamera();
    cam->SetPosition({ 0.0f, 3.0f, -5.0f });
    cam->SetTarget({ 0.0f, 1.0f,  0.0f });

    auto view = cam->GetViewMatrix();
    auto proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    // スカイドームをカメラに追従
    for (auto* obj : m_sceneObjects) {
        if (obj->tag == "Sky") {
            if (auto* tr = obj->GetComponent<Transform>())
                tr->position = cam->GetPosition();
        }
    }

    engine->GetRenderer()->BeginFrame();
    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        engine->GetRenderer()->DrawObject(m_sceneObjects[i], i, view, proj);
    }
    engine->GetRenderer()->EndFrame();
}

GameOverScene::~GameOverScene() {
    for (auto* obj : m_sceneObjects) delete obj;
    m_sceneObjects.clear();
}
